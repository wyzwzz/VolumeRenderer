//
// Created by wyz on 20-12-4.
//
#include"simple_volume_renderer.h"

void SimpleVolumeRenderer::setupVolume(const char *file_path)
{
    volume_manager->setupVolumeData(file_path);
    glGenTextures(1,&volume_tex);
    glBindTextureUnit(VOLUMEDATA_TEXTURE_BINDING,volume_tex);
    glBindTexture(GL_TEXTURE_3D,volume_tex);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
    auto dim=volume_manager->getVolumeDim();
    glTexImage3D(GL_TEXTURE_3D,0,GL_RED,dim[0],dim[1],dim[2],0,GL_RED,GL_UNSIGNED_BYTE,volume_manager->getVolumeData().data());
    std::cout<<volume_manager->getVolumeData().size()<<std::endl;
}

void SimpleVolumeRenderer::setupTransferFunc(std::map<double, std::array<double, 4>> color_setting)
{
    volume_manager->setupTransferFunc(color_setting);
    glGenTextures(1,&transfer_func_tex);
    glBindTextureUnit(TF_TEXTURE_BINDING,transfer_func_tex);
    glBindTexture(GL_TEXTURE_1D,transfer_func_tex);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,TF_DIM,0,GL_RGBA,GL_FLOAT,volume_manager->getTransferFunc(false).data());

    glGenTextures(1,&preInt_tf_tex);
    glBindTextureUnit(PREINT_TF_TEXTURE_BINDING,preInt_tf_tex);
    glBindTexture(GL_TEXTURE_2D,preInt_tf_tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,TF_DIM,TF_DIM,0,GL_RGBA,GL_FLOAT, volume_manager->getTransferFunc(true).data());
}

void SimpleVolumeRenderer::init()
{
    initGL();
    setupProxyCube(1.0,1.0,1.0);
    setupScreenQuad();
    setupRaycastPosFramebuffer();
}

void SimpleVolumeRenderer::render()
{

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
//    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//    glfwSetCursorPosCallback(window, mouse_callback);
//    glfwSetScrollCallback(window, scroll_callback);
//    glfwSetKeyCallback(window,keyboard_callback);
//    glfwSetMouseButtonCallback(window,mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
    glBindTextureUnit(ENTRYPOS_TEXTURE_BINDING,entrypos_tex);
    glBindTexture(GL_TEXTURE_2D,entrypos_tex);
    glTextureStorage2D(entrypos_tex,1,GL_RGBA32F,window_width,window_height);
    glBindImageTexture(ENTRYPOS_IMAGE_BINDING,entrypos_tex,0,GL_FALSE,0,GL_READ_ONLY,GL_RGBA32F);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,entrypos_tex,0);

    glGenRenderbuffers(1,&raycastpos_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER,raycastpos_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,window_width,window_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, raycastpos_rbo);

    glGenTextures(1,&exitpos_tex);
    glBindTextureUnit(EXITPOS_TEXTURE_BINDING,exitpos_tex);
    glBindTexture(GL_TEXTURE_2D,exitpos_tex);
    glTextureStorage2D(exitpos_tex,1,GL_RGBA32F,window_width,window_height);
    glBindImageTexture(EXITPOS_IMAGE_BINDING,exitpos_tex,0,GL_FALSE,0,GL_READ_ONLY,GL_RGBA32F);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,exitpos_tex,0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){
        throw std::runtime_error("Framebuffer object is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SimpleVolumeRenderer::setupController()
{

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
