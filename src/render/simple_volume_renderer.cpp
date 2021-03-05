#include"simple_volume_renderer.h"
#include<utils/help_gl.h>
#include<data/simple_volume_manager.h>
SimpleVolumeRenderer::SimpleVolumeRenderer():window_width(1200),window_height(900)
{
    initGL();
    volume_manager=std::make_unique<SimpleVolumeManager>();
    raycastpos_shader=std::make_unique<sv::Shader>(RAYCAST_POS_VERT,RAYCAST_POS_FRAG);
    raycasting_shader=std::make_unique<sv::Shader>(RAYCASTING_VERT,RAYCASTING_FRAG);
}

void SimpleVolumeRenderer::setupVolume(const char *file_path)
{
    volume_manager->setupVolumeData(file_path);
    glGenTextures(1,&volume_tex);
    glBindTexture(GL_TEXTURE_3D,volume_tex);
    glBindTextureUnit(VOLUMEDATA_TEXTURE_BINDING,volume_tex);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
    auto dim=volume_manager->getVolumeDim();
    std::cout<<"dim: "<<dim[0]<<" "<<dim[1]<<" "<<dim[2]<<std::endl;
    glTexImage3D(GL_TEXTURE_3D,0,GL_RED,dim[0],dim[1],dim[2],0,GL_RED,GL_UNSIGNED_BYTE,volume_manager->getVolumeData().data());
    std::cout<<volume_manager->getVolumeData().size()<<std::endl;
    GL_CHECK
}

void SimpleVolumeRenderer::setupTransferFunc(std::map<uint8_t, std::array<double, 4>> color_setting)
{
    volume_manager-> setupTransferFunc(color_setting);
    glGenTextures(1,&transfer_func_tex);
    glBindTexture(GL_TEXTURE_1D,transfer_func_tex);
    GL_EXPR(glBindTextureUnit(TF_TEXTURE_BINDING,transfer_func_tex));
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,TF_DIM,0,GL_RGBA,GL_FLOAT,volume_manager->getTransferFunc(false).data());

    glGenTextures(1,&preInt_tf_tex);
    glBindTexture(GL_TEXTURE_2D,preInt_tf_tex);
    glBindTextureUnit(PREINT_TF_TEXTURE_BINDING,preInt_tf_tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,TF_DIM,TF_DIM,0,GL_RGBA,GL_FLOAT, volume_manager->getTransferFunc(true).data());
}

void SimpleVolumeRenderer::init()
{
    setupController();

    setupProxyCube(1.0,1.0,1.0);

    setupScreenQuad();

    setupRaycastPosFramebuffer();

    setupShaderUniform();
}
void printmat4(glm::mat4 m)
{
    std::cout<<m[0][0]<<" "<<m[0][1]<<" "<<m[0][2]<<" "<<m[0][3]<<std::endl
             <<m[1][0]<<" "<<m[1][1]<<" "<<m[1][2]<<" "<<m[1][3]<<std::endl
             <<m[2][0]<<" "<<m[2][1]<<" "<<m[2][2]<<" "<<m[2][3]<<std::endl
             <<m[3][0]<<" "<<m[3][1]<<" "<<m[3][2]<<" "<<m[3][3]<<std::endl;
}
void SimpleVolumeRenderer::setupShaderUniform()
{
    raycasting_shader->use();
    raycasting_shader->setInt("transfer_func",TF_TEXTURE_BINDING);
    raycasting_shader->setInt("preInt_transferfunc",PREINT_TF_TEXTURE_BINDING);
    raycasting_shader->setInt("volume_data",VOLUMEDATA_TEXTURE_BINDING);


    raycasting_shader->setFloat("ka",0.5f);
    raycasting_shader->setFloat("kd",0.8f);
    raycasting_shader->setFloat("shininess",100.0f);
    raycasting_shader->setFloat("ks",1.0f);
    raycasting_shader->setVec3("light_direction",glm::normalize(glm::vec3(-1.0f,-1.0f,-1.0f)));

    raycasting_shader->setFloat("step",1.0f/256*0.3f);

}

void SimpleVolumeRenderer::render()
{
    GL_CHECK
    while(!glfwWindowShouldClose(window)){
        float current_frame=glfwGetTime();
        sv::Controller::delta_time=current_frame-sv::Controller::last_frame;
        sv::Controller::last_frame=current_frame;
//        std::cout<<"fps: "<<1.0f/sv::Controller::delta_time<<std::endl;

        glfwPollEvents();
        sv::Controller::processInput(window);

        glm::mat4 model=glm::mat4(1.0f);
        glm::mat4 view=sv::Controller::getCamera().getViewMatrix();

        glm::mat4 projection=glm::perspective(glm::radians(sv::Controller::getCamera().getZoom()),
                                                           (float)window_width/(float)window_height,
                                                           0.1f,50.0f);
        glm::mat4 mvp=projection*view*model;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        raycastpos_shader->use();
        raycastpos_shader->setMat4("MVPMatrix",mvp);
        glBindFramebuffer(GL_FRAMEBUFFER,raycastpos_fbo);
        glBindVertexArray(proxy_cube_vao);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);

        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
        glDisable(GL_CULL_FACE);
        glBindFramebuffer(GL_FRAMEBUFFER,0);

        raycasting_shader->use();

        glBindVertexArray(screen_quad_vao);
        glDrawArrays(GL_TRIANGLES,0,6);

        GL_CHECK

        glfwSwapBuffers(window);
    }
    glfwTerminate();
}

void SimpleVolumeRenderer::initGL()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSwapInterval(1);
    window = glfwCreateWindow(window_width, window_height, "Volume Render", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return ;
    }

    glfwMakeContextCurrent(window);


//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glEnable(GL_DEPTH_TEST);
}

void SimpleVolumeRenderer::setupProxyCube(GLfloat x, GLfloat y, GLfloat z)
{
    proxy_cube_x_len=x;proxy_cube_y_len=y;proxy_cube_z_len=z;
    proxy_cube_vertices[0]={0.0f,0.0f,0.0f};
    proxy_cube_vertices[1]={x,0.0f,0.0f};
    proxy_cube_vertices[2]={x,y,0.0f};
    proxy_cube_vertices[3]={0.0f,y,0.0f};
    proxy_cube_vertices[4]={0.0f,0.0f,z};
    proxy_cube_vertices[5]={x,0.0f,z};
    proxy_cube_vertices[6]={x,y,z};
    proxy_cube_vertices[7]={0.0f,y,z};

    proxy_cube_vertex_indices={0,1,2,0,2,3,
                               0,4,1,4,5,1,
                               1,5,6,6,2,1,
                               6,7,2,7,3,2,
                               7,4,3,3,4,0,
                               4,7,6,4,6,5};

    glGenVertexArrays(1,&proxy_cube_vao);
    glGenBuffers(1,&proxy_cube_vbo);
    glGenBuffers(1,&proxy_cube_ebo);
    glBindVertexArray(proxy_cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER,proxy_cube_vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(proxy_cube_vertices),proxy_cube_vertices.data(),GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,proxy_cube_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(proxy_cube_vertex_indices),proxy_cube_vertex_indices.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void SimpleVolumeRenderer::setupScreenQuad()
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

void SimpleVolumeRenderer::setupRaycastPosFramebuffer()
{
    glGenFramebuffers(1,&raycastpos_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,raycastpos_fbo);

    glGenTextures(1,&entrypos_tex);
    glBindTexture(GL_TEXTURE_2D,entrypos_tex);
    glBindTextureUnit(ENTRYPOS_TEXTURE_BINDING,entrypos_tex);
    glTextureStorage2D(entrypos_tex,1,GL_RGBA32F,window_width,window_height);
    glBindImageTexture(ENTRYPOS_IMAGE_BINDING,entrypos_tex,0,GL_FALSE,0,GL_READ_ONLY,GL_RGBA32F);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,entrypos_tex,0);
    GL_CHECK;
    glGenRenderbuffers(1,&raycastpos_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER,raycastpos_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,window_width,window_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, raycastpos_rbo);

    glGenTextures(1,&exitpos_tex);
    glBindTexture(GL_TEXTURE_2D,exitpos_tex);
    glBindTextureUnit(EXITPOS_TEXTURE_BINDING,exitpos_tex);
    glTextureStorage2D(exitpos_tex,1,GL_RGBA32F,window_width,window_height);
    glBindImageTexture(EXITPOS_IMAGE_BINDING,exitpos_tex,0,GL_FALSE,0,GL_READ_ONLY,GL_RGBA32F);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,exitpos_tex,0);
    GL_CHECK
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){
        throw std::runtime_error("Framebuffer object is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_CHECK
}

void SimpleVolumeRenderer::setupController()
{
    glfwSetFramebufferSizeCallback(window, sv::Controller::FramebufferSizeCallback);
    glfwSetCursorPosCallback(window, sv::Controller::CursorPosCallback);
    glfwSetScrollCallback(window, sv::Controller::ScrollCallback);
    glfwSetKeyCallback(window,sv::Controller::KeyCallback);
    glfwSetMouseButtonCallback(window,sv::Controller::MouseButtonCallback);
}

void SimpleVolumeRenderer::deleteGLResource()
{
    glDeleteTextures(1,&transfer_func_tex);
    glDeleteTextures(1,&preInt_tf_tex);
    glDeleteTextures(1,&entrypos_tex);
    glDeleteTextures(1,&exitpos_tex);
    glDeleteTextures(1,&volume_tex);
    glDeleteVertexArrays(1,&proxy_cube_vao);
    glDeleteBuffers(1,&proxy_cube_vbo);
    glDeleteBuffers(1,&proxy_cube_ebo);
    glDeleteVertexArrays(1,&screen_quad_vao);
    glDeleteBuffers(1,&screen_quad_vbo);
    glDeleteFramebuffers(1,&raycastpos_fbo);
    glDeleteRenderbuffers(1,&raycastpos_rbo);
}

SimpleVolumeRenderer::~SimpleVolumeRenderer()
{
    deleteGLResource();
}