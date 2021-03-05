//
// Created by wyz on 20-12-4.
//
#include"block_volume_renderer.h"
#include<utils/help_gl.h>
#include<data/block_volume_manager.h>
#include<cudaGL.h>

BlockVolumeRenderer::BlockVolumeRenderer()
: window_width(1200), window_height(900),
  block_length(0), vol_tex_block_nx(0), vol_tex_block_ny(0),
  vol_tex_num(0),
  block_dim({0,0,0}), cu_context(nullptr)
{
    initGL();
    initCUDA();
    volume_manager=std::make_unique<BlockVolumeManager>();
    camera=std::make_unique<sv::RayCastOrthoCamera>(glm::vec3(1.f,1.f,1.f),window_width/2,window_height/2);

}
void BlockVolumeRenderer::setupVolume(const char *file_path)
{
    volume_manager->setupVolumeData(file_path);


}

void BlockVolumeRenderer::setupTransferFunc(std::map<uint8_t, std::array<double, 4>> color_setting)
{
    volume_manager-> setupTransferFunc(color_setting);
    glGenTextures(1,&transfer_func_tex);
    glBindTexture(GL_TEXTURE_1D,transfer_func_tex);
//    GL_EXPR(glBindTextureUnit(TF_TEXTURE_BINDING,transfer_func_tex));
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,TF_DIM,0,GL_RGBA,GL_FLOAT,volume_manager->getTransferFunc(false).data());

    glGenTextures(1,&preInt_tf_tex);
    glBindTexture(GL_TEXTURE_2D,preInt_tf_tex);
//    glBindTextureUnit(PREINT_TF_TEXTURE_BINDING,preInt_tf_tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,TF_DIM,TF_DIM,0,GL_RGBA,GL_FLOAT, volume_manager->getTransferFunc(true).data());
}

void BlockVolumeRenderer::init()
{

    setupController();

    setupVolumeDataInfo();
    setupGPUMemory();

    createGLResource();
    createGLTexture();
    createGLSampler();
    createCUgraphicsResource();
    createVolumeTexManager();
    createVirtualBoxes();
    createMappingTable();

    setupShaderUniform();
}

void BlockVolumeRenderer::bindGLTextureUnit()
{

}

void BlockVolumeRenderer::setupShaderUniform()
{
    raycasting_shader->use();

    raycasting_shader->setInt("window_width",window_width);
    raycasting_shader->setInt("window_height",window_height);
    raycasting_shader->setVec3("view_pos",camera->view_pos);
    raycasting_shader->setVec3("view_direction",camera->view_direction);
    raycasting_shader->setVec3("view_right",camera->right);
    raycasting_shader->setVec3("view_up",camera->up);
    raycasting_shader->setFloat("view_right_space",camera->space_x);
    raycasting_shader->setFloat("view_up_space",camera->space_y);
    raycasting_shader->setFloat("view_right_space",0.01f);
    raycasting_shader->setFloat("view_up_space",0.01f);
    raycasting_shader->setFloat("step",1.f);
}

std::function<void(GLFWwindow*,float)> process_input;
void BlockVolumeRenderer::render()
{
    print_args();
    GL_CHECK
    float last_frame_t=0.f;
    while(!glfwWindowShouldClose(window)){

        float current_frame_t=glfwGetTime();
        float delta_t=current_frame_t-last_frame_t;
        last_frame_t=current_frame_t;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        //1.event process
        glfwPollEvents();
        process_input(window,delta_t);
        auto obb=camera->getOBB();
        updateCurrentBlocks(obb);

        //2.calculate new pass render needing blocks
        //sort these blocks by distance to viewport
        volume_manager->setupBlockReqInfo(getBlockRequestInfo());

        //3.update blocks: uncompress and loading
        //multi-thread uncompress and copy from device to cuda array
        //if a block is waiting for loading but turn to no need now, stop its task
        //while copying from device to texture, device can't be accessed by other thread or task
        BlockDesc block;
        while(volume_manager->getBlock(block)) {

//            print_array(block.block_index);

            copyDeviceToTexture(block.data,block.block_index);

            updateMappingTable();

            //stupid method
            ((BlockVolumeManager*)(volume_manager.get()))->updateMemoryPool(block.data);
            //4.update current blocks' status: cached and empty; update block map table
            //update every 16ms(fixed time interval)

        }

        //5.render a frame
        //ray stop if sample at an empty block
        updateCameraUniform();
        render_frame();

        //6.final
        glfwSwapBuffers(window);
    }

}
void BlockVolumeRenderer::updateCameraUniform()
{
    raycasting_shader->use();
    raycasting_shader->setVec3("view_pos",camera->view_pos);
    raycasting_shader->setVec3("view_direction",camera->view_direction);
    raycasting_shader->setVec3("view_right",camera->right);
    raycasting_shader->setVec3("view_up",camera->up);
}

void BlockVolumeRenderer::render_frame()
{
    raycasting_shader->use();
    glBindVertexArray(screen_quad_vao);

    glDrawArrays(GL_TRIANGLES,0,6);

}



void BlockVolumeRenderer::deleteGLTexture()
{
    for(size_t i=0;i<volume_texes.size();i++){
        CUDA_DRIVER_API_CALL(cuGraphicsUnregisterResource(cu_resources[i]));
    }
    glDeleteTextures(volume_texes.size(),volume_texes.data());
}

void BlockVolumeRenderer::initCUDA()
{
    CUDA_DRIVER_API_CALL(cuInit(0));
    CUdevice cuDevice=0;
    CUDA_DRIVER_API_CALL(cuDeviceGet(&cuDevice, 0));
    CUDA_DRIVER_API_CALL(cuCtxCreate(&cu_context,0,cuDevice));
}
/**
 * index, CUdeviceptr
 */
void BlockVolumeRenderer::copyDeviceToTexture(CUdeviceptr ptr,std::array<uint32_t,3> idx)
{
    //inspect if idx is still in current_blocks


    //getTexturePos() set valid=false cached=false

    std::array<uint32_t,3> tex_pos_index;
    bool cached=getTexturePos(idx,tex_pos_index);
    //if cached, update map-table
//    if(cached)
//        std::cout<<"find cached block"<<std::endl;

    std::cout<<__FUNCTION__ <<std::endl;
//    print_array(tex_pos_index);

    if(!cached){
        assert(tex_pos_index[2] < vol_tex_num && tex_pos_index[0] < vol_tex_block_nx
               && tex_pos_index[1]<vol_tex_block_ny);

        CUDA_DRIVER_API_CALL(cuGraphicsMapResources(1, &cu_resources[tex_pos_index[2]], 0));

        CUarray cu_array;

        CUDA_DRIVER_API_CALL(cuGraphicsSubResourceGetMappedArray(&cu_array,cu_resources[tex_pos_index[2]],0,0));



        CUDA_MEMCPY3D m = { 0 };
        m.srcMemoryType=CU_MEMORYTYPE_DEVICE;
        m.srcDevice=ptr;


        m.dstMemoryType=CU_MEMORYTYPE_ARRAY;
        m.dstArray=cu_array;
        m.dstXInBytes=tex_pos_index[0]*block_length;
        m.dstY=tex_pos_index[1]*block_length;
        m.dstZ=0;

        m.WidthInBytes=block_length;
        m.Height=block_length;
        m.Depth=block_length;

        CUDA_DRIVER_API_CALL(cuMemcpy3D(&m));

        CUDA_DRIVER_API_CALL(cuGraphicsUnmapResources(1, &cu_resources[tex_pos_index[2]], 0));
    }
    //update volume_tex_manager

    for(auto& it:volume_tex_manager){
        if(it.pos_index==tex_pos_index){
            if(cached){
                assert(it.cached==true && it.block_index==idx);
            }
            it.block_index=idx;
            it.valid=true;
            it.cached=true;
        }
    }

    //update mapping_table
    uint32_t flat_idx=idx[2]*block_dim[0]*block_dim[1]+idx[1]*block_dim[0]+idx[0];
    mapping_table[flat_idx*4+0]=tex_pos_index[0];
    mapping_table[flat_idx*4+1]=tex_pos_index[1];
    mapping_table[flat_idx*4+2]=tex_pos_index[2];
    mapping_table[flat_idx*4+3]=1;
}
void BlockVolumeRenderer::updateMappingTable() {

    glNamedBufferSubData(mapping_table_ssbo,0,mapping_table.size(),mapping_table.data());

}
void BlockVolumeRenderer::initGL()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(window_width, window_height, "Volume Render", NULL, NULL);
    if (window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glEnable(GL_DEPTH_TEST);
}

void BlockVolumeRenderer::createGLResource() {
    std::cout<<__FUNCTION__<<std::endl;
    //create shader
    raycasting_shader=std::make_unique<sv::Shader>(Block_Raycasting_Shader_V,Block_Raycasting_Shader_F);
    createScreenQuad();

}
void BlockVolumeRenderer::createScreenQuad()
{
    screen_quad_vertices={
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1,&screen_quad_vao);
    glGenBuffers(1,&screen_quad_vbo);
    glBindVertexArray(screen_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER,screen_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(screen_quad_vertices),screen_quad_vertices.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}



void BlockVolumeRenderer::updateCurrentBlocks(const sv::OBB &view_box)
{
    auto aabb=view_box.getAABB();
    std::unordered_set<sv::AABB,Myhash> current_intersect_blocks;
//    print(aabb);
    for(auto& it:virtual_blocks){
        if(aabb.intersect(it)){
            current_intersect_blocks.insert(it);
//            print(it);
        }
    }
//    std::cout<<current_intersect_blocks.size()<<std::endl;

    assert(new_need_blocks.empty());
    assert(no_need_blocks.empty());
    for(auto& it:current_intersect_blocks){
        if(current_blocks.find(it)==current_blocks.cend()){
            new_need_blocks.insert(it);
        }
    }

    for(auto& it:current_blocks){
        if(current_intersect_blocks.find(it)==current_intersect_blocks.cend()){
            no_need_blocks.insert(it);
        }
    }
//    std::cout<<"new size: "<<new_need_blocks.size()<<std::endl;
//    std::cout<<"no size: "<<no_need_blocks.size()<<std::endl;
    current_blocks=std::move(current_intersect_blocks);

    for(auto& it:volume_tex_manager){
        auto t=sv::AABB(it.block_index);
        //not find
        if(current_blocks.find(t)==current_blocks.cend()){
            it.valid=false;
            //update mapping_table
//            print_array(it.block_index);
            if(it.block_index[0]!=INVALID){
                uint32_t flat_idx=it.block_index[2]*block_dim[0]*block_dim[1]+it.block_index[1]*block_dim[0]+it.block_index[0];
                mapping_table[flat_idx*4+3]=0;
            }
        }
        else{
            assert(it.valid==true || (it.valid==false && it.cached==true));
        }
    }


}

void BlockVolumeRenderer::createVirtualBoxes()
{
    std::cout<<__FUNCTION__<<std::endl;
    for(uint32_t z=0;z<block_dim[2];z++){
        for(uint32_t y=0;y<block_dim[1];y++){
            for(uint32_t x=0;x<block_dim[0];x++){
                virtual_blocks.emplace_back(glm::vec3(x*block_length,y*block_length,z*block_length),
                                            glm::vec3((x+1)*block_length,(y+1)*block_length,(z+1)*block_length),
                                            std::array<uint32_t,3>{x,y,z});
            }
        }
    }

}
void BlockVolumeRenderer::createMappingTable() {
    mapping_table.assign(block_dim[0]*block_dim[1]*block_dim[2]*4,0);
    glGenBuffers(1,&mapping_table_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,mapping_table_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,mapping_table.size(),mapping_table.data(),GL_DYNAMIC_READ);
    //binding point = 0
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,mapping_table_ssbo);
}
/**
 * call after getting volume textures size
 */
void BlockVolumeRenderer::createVolumeTexManager() {
    std::cout<<__FUNCTION__<<std::endl;
    assert(vol_tex_num == volume_texes.size());
    for(uint32_t i=0;i<vol_tex_block_nx;i++){
        for(uint32_t j=0;j<vol_tex_block_ny;j++){
            for(uint32_t k=0; k < vol_tex_num; k++){
                BlockTableItem item;
                item.pos_index={i,j,k};
                item.block_index={INVALID,INVALID,INVALID};
                item.valid=false;
                item.cached=false;
                volume_tex_manager.push_back(std::move(item));
            }
        }
    }
}
bool BlockVolumeRenderer::getTexturePos(const std::array<uint32_t,3>& idx,std::array<uint32_t, 3>& pos)
{

    for(auto&it :volume_tex_manager){
        if(it.block_index==idx && it.cached && !it.valid){
            pos=it.pos_index;
            return true;
        }
        else if(it.block_index==idx && !it.cached && !it.valid){
            pos=it.pos_index;
            return false;
        }
    }
    for(auto& it:volume_tex_manager){
        if(!it.valid && !it.cached){
            pos=it.pos_index;
            return false;
        }
    }
    for(auto& it:volume_tex_manager){
        if(!it.valid){
            pos=it.pos_index;
            return false;
        }
    }
    throw std::runtime_error("not find empty texture pos");

}

void BlockVolumeRenderer::createCUgraphicsResource() {

    assert(vol_tex_num == volume_texes.size() && vol_tex_num != 0);
    cu_resources.resize(volume_texes.size());
    for(int i=0;i<volume_texes.size();i++){
        CUDA_DRIVER_API_CALL(cuGraphicsGLRegisterImage(&cu_resources[i], volume_texes[i],GL_TEXTURE_3D, CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD));
    }

}

void BlockVolumeRenderer::deleteCUgraphicsResource() {
    assert(volume_texes.size()==cu_resources.size() && vol_tex_num == volume_texes.size() && vol_tex_num != 0);
    for(int i=0;i<cu_resources.size();i++){
        CUDA_DRIVER_API_CALL(cuGraphicsUnregisterResource(cu_resources[i]));
    }
}

void BlockVolumeRenderer::createGLTexture() {
    assert(block_length && vol_tex_block_nx && vol_tex_block_ny && vol_tex_num);
    assert(volume_texes.size()==0);
    volume_texes.assign(vol_tex_num, 0);
    std::cout<<__FUNCTION__ <<std::endl;
    glCreateTextures(GL_TEXTURE_3D, vol_tex_num, volume_texes.data());
    for(int i=0; i < vol_tex_num; i++){
        glTextureStorage3D(volume_texes[i],1,GL_R8,vol_tex_block_nx*block_length,
                                                   vol_tex_block_ny*block_length,
                                                   block_length);
    }

}
void BlockVolumeRenderer::createGLSampler() {
    glCreateSamplers(1,&gl_sampler);
    glSamplerParameterf(gl_sampler,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glSamplerParameterf(gl_sampler,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glSamplerParameterf(gl_sampler,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
    glSamplerParameterf(gl_sampler,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glSamplerParameterf(gl_sampler,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
}

BlockRequestInfo BlockVolumeRenderer::getBlockRequestInfo() {
    BlockRequestInfo request;
    for(auto& it:new_need_blocks){
        request.request_blocks_queue.push_back(it.index);
    }
    for(auto& it:no_need_blocks){
        request.noneed_blocks_queue.push_back(it.index);
    }
//    std::cout<<"new_need_blocks size: "<<new_need_blocks.size()<<std::endl;
    new_need_blocks.clear();
    no_need_blocks.clear();
    return request;

}

void BlockVolumeRenderer::setupVolumeDataInfo() {
    auto volume_data_info=volume_manager->getVolumeDataInfo();
    this->block_length=volume_data_info.block_length;
    this->block_dim={volume_data_info.block_dim_x,volume_data_info.block_dim_y,volume_data_info.block_dim_z};

}

void BlockVolumeRenderer::setupGPUMemory() {
    int nx=(window_width+block_length-1)/block_length*2;
    int ny=(window_height+block_length-1)/block_length*2;
    vol_tex_block_nx=nx;
    vol_tex_block_ny=ny;
    vol_tex_num=3;
}

void BlockVolumeRenderer::print_args()
{
    std::cout << "[Block Volume Renderer Args]:"
              << "\n\tblock_length: " << block_length
              << "\n\tblock_dim: (" << block_dim[0] << " " << block_dim[1] << " " << block_dim[2] << ")"
              << "\n\tvirtual_blocks size: " << virtual_blocks.size()
              << "\n\tmapping_table size: " << mapping_table.size()
              << "\n\tvol_tex_block_nx: " << vol_tex_block_nx
              << "\n\tvol_tex_block_ny: " << vol_tex_block_ny
              << "\n\tview_depth_level: " << vol_tex_num
    <<"\n\tvolume_texes size: "<<volume_texes.size()
    <<"\n\tvolume_tex_manager size: "<<volume_tex_manager.size()
    <<"\n\tcu_resources size: "<<cu_resources.size()
    <<"\n\twindow_width && window_height: "<<window_width<<" "<<window_height
    <<std::endl;
}

BlockVolumeRenderer::~BlockVolumeRenderer() {
    deleteCUgraphicsResource();
    deleteGLTexture();

}


std::function<void(GLFWwindow *window, int width, int height)> framebuffer_resize_callback;
void glfw_framebuffer_resize_callback(GLFWwindow *window, int width, int height){
    framebuffer_resize_callback(window,width,height);
}
std::function<void(GLFWwindow* window, int button, int action, int mods)> mouse_button_callback;
void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    mouse_button_callback(window,button,action,mods);
}
std::function<void(GLFWwindow *window, double xpos, double ypos)> mouse_move_callback;
void glfw_mouse_move_callback(GLFWwindow *window, double xpos, double ypos){
    mouse_move_callback(window,xpos,ypos);
}
std::function<void(GLFWwindow *window, double xoffset, double yoffset)> scroll_callback;
void glfw_scroll_callback(GLFWwindow *window, double xoffset, double yoffset){
    scroll_callback(window,xoffset,yoffset);
}
std::function<void(GLFWwindow *window, int key, int scancode, int action, int mods)> keyboard_callback;
void glfw_keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
    keyboard_callback(window,key,scancode,action,mods);
}

void BlockVolumeRenderer::setupController()
{
    static bool clicked;
    framebuffer_resize_callback=[&](GLFWwindow *window, int width, int height){
        std::cout<<__FUNCTION__ <<std::endl;
    };
    mouse_button_callback=[&](GLFWwindow* window, int button, int action, int mods){
        if(button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_PRESS){
            print("mouse button left pressed");
        }
        else if(button==GLFW_MOUSE_BUTTON_RIGHT && action==GLFW_PRESS){
            print("mouse button right pressed");
        }
    };
    mouse_move_callback=[&](GLFWwindow *window, double xpos, double ypos){
        static double last_x,last_y;
        static bool first=true;
        if(first){
            first=false;
            last_x=xpos;
            last_y=ypos;
        }
        double delta_x=xpos-last_x;
        double delta_y=last_y-ypos;
        last_x=xpos;
        last_y=ypos;
        if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS){

        }
        else if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS){
            camera->processMouseMove(delta_x,delta_y);
        }
    };
    scroll_callback=[&](GLFWwindow *window, double xoffset, double yoffset){
        camera->processMouseScroll(yoffset);
    };
    keyboard_callback=[&](GLFWwindow *window, int key, int scancode, int action, int mods){
        if(key==GLFW_KEY_ESCAPE && action==GLFW_PRESS){
            glfwSetWindowShouldClose(window, true);
        }
        if(key==GLFW_KEY_F && action==GLFW_PRESS){
            camera->processKeyForArg(sv::CameraDefinedKey::MOVE_FASTER);
        }

    };
    glfwSetFramebufferSizeCallback(window,glfw_framebuffer_resize_callback);
    glfwSetMouseButtonCallback(window,glfw_mouse_button_callback);
    glfwSetCursorPosCallback(window,glfw_mouse_move_callback);
    glfwSetScrollCallback(window,glfw_scroll_callback);
    glfwSetKeyCallback(window,glfw_keyboard_callback);

    process_input=[&](GLFWwindow *window,float delta_time){
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera->processMovementByKey(sv::CameraMoveDirection::FORWARD, delta_time);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera->processMovementByKey(sv::CameraMoveDirection::BACKWARD, delta_time);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera->processMovementByKey(sv::CameraMoveDirection::LEFT, delta_time);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera->processMovementByKey(sv::CameraMoveDirection::RIGHT, delta_time);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera->processMovementByKey(sv::CameraMoveDirection::UP, delta_time);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera->processMovementByKey(sv::CameraMoveDirection::DOWN, delta_time);
    };
}










