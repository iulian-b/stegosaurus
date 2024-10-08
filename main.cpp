/*********************************/
/* StegoSaurus                   */
/* Program de Stegannaliza. 2022 */
/*********************************/
/* Iulian Ionel Bocse            */
/* iulian@ibocse.com             */
/*********************************/

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imfilebrowser.h"

// STD
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdlib.h>
#include <cstdint>

// ArgParses
#include "argparse.hpp"

// ImageMagick
#include <Magick++.h>

// OpenCV
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

// Exiftool
#include "ExifTool.h"
#include "ExifToolPipe.h"
#include "TagInfo.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> 

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace fs = std::filesystem;
using namespace std;
// using namespace cv;
// using namespace Magick;

// Global Elite
std::string EXTENTION;
std::string DecodedMessage;
bool DEBUG = false;
bool COMPARE = false;
bool ENCODE_LSB = false;
bool DECODE_LSB = false;
int POOL_SIZE = 39;

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height){
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

// Return file extention from given path
std::string GetExtention(std::string s){
    reverse(s.begin(), s.end());
    size_t pos = s.find(".");
    
    s = s.erase(pos,s.length());
    reverse(s.begin(), s.end());

    return s;
}

std::string GetName(std::string s){
    while (s.find("/") < 299){
        size_t pos = s.find("/");
        if (pos == 0){
            s = s.erase(0,1);
            continue;
        }
        s = s.erase(0,pos);
    }
    return "./" + s;
}

bool isBitSet(char C, int Position) {
    C = C >> Position;
    bool Output = C & 1 ? true : false;
    return Output;
}

void EncodeLSB(char secretC[1024], std::string Path){
    std::ofstream tmpFile("secret");
    std::string secretS(secretC);
    tmpFile<<secretS;
    tmpFile.close();

    // Exec Encode
    std::string cmd = "./encode " + Path + " secret lsb_encoded.png";
    FILE* fp = popen(cmd.c_str(),"r");    
    if(DEBUG) std::cout<<"[LSB-ENCODE] Message Encoded into new file `lsb_encoded.png`";    
}

std::string DecodeLSB(std::string Selection){
    std::string Cmd = "./decode " + Selection;
    char Path [Cmd.length() + 1];
    strcpy(Path,Cmd.c_str());
    FILE *fp = popen (Path,"r");

    // Read Extracted Message
    system("sleep 1");
    std::string read, Output; 
    std::ifstream Extract("StegoSaurus-LSB-Extract");
    while (getline (Extract, read)){ 
        Output = Output + read; 
    }
    Extract.close();

    return Output;
}

bool isFileEmpty(std::ifstream& pFile){
    return pFile.peek() == std::ifstream::traits_type::eof();
}

struct StegImage{
    GLuint texture = 0;
    std::string path;
} ImagePool[40];


void GenerateVariants(std::string PathS, char Path[], char PathComp[], char ext[], StegImage (&ImagePool)[40], Magick::Image img, Magick::Image comp){
    EXTENTION = ext;
    Magick::Image Extr, Var;
    Var = img;
    
    // TEMPLATE:
    // Image.AppltFunction(Parameters); Image.WriteToDirectory(); ImagePool[n] = NewVariantPath; Image = ResetImageBackToOriginalForm;

    // Separate RGBA
    Extr = Var.separate(Magick::RedChannel); Extr.write(("./pic/Red_Channel." + EXTENTION)); ImagePool[1].path = "./pic/Red_Channel." + EXTENTION;
    Extr = Var.separate(Magick::GreenChannel); Extr.write(("./pic/Green_Channel." + EXTENTION)); ImagePool[2].path = "./pic/Green_Channel." + EXTENTION;
    Extr = Var.separate(Magick::BlueChannel); Extr.write(("./pic/Blue_Channel." + EXTENTION)); ImagePool[3].path = "./pic/Blue_Channel." + EXTENTION;
    Extr = Var.separate(Magick::AlphaChannel); Extr.write(("./pic/Alpha_Channel." + EXTENTION)); ImagePool[4].path = "./pic/Alpha_Channel." + EXTENTION;
    //Extr = Var.separate(Magick::MatteChannel); Extr.write(("./pic2/Matte_Channel." + EXTENTION));         NOTE: Same as Alpha??
    
    // Evaluate RGB
    Var.levelChannel(Magick::RedChannel,100,1,1); Var.write(("./pic/No_Red_Channel.") + EXTENTION); Var = img; ImagePool[5].path = "./pic/No_Red_Channel." + EXTENTION;
    Var.levelChannel(Magick::GreenChannel,100,1,1); Var.write(("./pic/No_Green_Channel.") + EXTENTION); Var = img; ImagePool[6].path = "./pic/No_Green_Channel." + EXTENTION;
    Var.levelChannel(Magick::BlueChannel,100,1,1); Var.write(("./pic/No_Blue_Channel.") + EXTENTION); Var = img; ImagePool[7].path = "./pic/No_Blue_Channel." + EXTENTION;

    // Threshold
    Var.threshold( 35000.0 );  Var.write(("./pic/Threshold_35." + EXTENTION)); Var = img; ImagePool[8].path = "./pic/Threshold_35." + EXTENTION;
    Var.threshold( 45000.0 );  Var.write(("./pic/Threshold_45." + EXTENTION)); Var = img; ImagePool[9].path = "./pic/Threshold_45." + EXTENTION;
    Var.threshold( 55000.0 );  Var.write(("./pic/Threshold_55." + EXTENTION)); Var = img; ImagePool[10].path = "./pic/Threshold_55." + EXTENTION;

    // RGB Planes
    Var.levelChannel(Magick::RedChannel, 5.0, 100.0, 1.0); Var.write(("./pic/Red_Plane." + EXTENTION)); Var = img; ImagePool[11].path = "./pic/Red_Plane." + EXTENTION;
    Var.levelChannel(Magick::GreenChannel, 5.0, 100.0, 1.0); Var.write(("./pic/Green_Plane." + EXTENTION)); Var = img; ImagePool[12].path = "./pic/Green_Plane." + EXTENTION;
    Var.levelChannel(Magick::BlueChannel, 5.0, 100.0, 1.0); Var.write(("./pic/Blue_Plane." + EXTENTION)); Var = img; ImagePool[13].path = "./pic/Blue_Plane." + EXTENTION;
    //Extr = Var.separate(Magick::RedChannel); Extr.level(5.0, 100.0, 1.0); Extr.write(("./pic2/test2." + EXTENTION)); Var = img;  NOTE: Not workerino
    
    // Gamma
    Var.gamma(5);  Var.write(("./pic/Gamma_5." + EXTENTION)); Var = img; ImagePool[14].path = "./pic/Gamma_5." + EXTENTION;
    Var.gamma(10.0);  Var.write(("./pic/Gamma_10." + EXTENTION)); Var = img; ImagePool[15].path = "./pic/Gamma_10." + EXTENTION;
    Var.gamma(25.0);  Var.write(("./pic/Gamma_25." + EXTENTION)); Var = img; ImagePool[16].path = "./pic/Gamma_25." + EXTENTION;
    Var.gamma(40.0);  Var.write(("./pic/Gamma_40." + EXTENTION)); Var = img; ImagePool[17].path = "./pic/Gamma_40." + EXTENTION;

    // Color Maps
    Var.cycleColormap(10);  Var.write(("./pic/ColorMap_Plus_10." + EXTENTION)); Var = img; ImagePool[18].path = "./pic/ColorMap_Plus_10." + EXTENTION;
    Var.cycleColormap(50);  Var.write(("./pic/ColorMap_Plus_50." + EXTENTION)); Var = img; ImagePool[19].path = "./pic/ColorMap_Plus_50." + EXTENTION;
    Var.cycleColormap(100);  Var.write(("./pic/ColorMap_Plus_100." + EXTENTION)); Var = img; ImagePool[20].path = "./pic/ColorMap_Plus_100." + EXTENTION;
    Var.cycleColormap(-10);  Var.write(("./pic/ColorMap_Minus_10." + EXTENTION)); Var = img; ImagePool[21].path = "./pic/ColorMap_Minus_10." + EXTENTION;
    Var.cycleColormap(-50);  Var.write(("./pic/ColorMap_Minus_50." + EXTENTION)); Var = img; ImagePool[22].path = "./pic/ColorMap_Minus_50." + EXTENTION;
    Var.cycleColormap(-100);  Var.write(("./pic/ColorMap_Minus_100." + EXTENTION)); Var = img; ImagePool[23].path = "./pic/ColorMap_Minus_100." + EXTENTION;
          
    // Grayscale
    Var.quantizeColorSpace( Magick::GRAYColorspace ); Var.quantizeColors( 2 ); Var.quantizeDither( false ); Var.quantize( ); Var.write(("./pic/Monochrome.") + EXTENTION); Var = img; ImagePool[24].path = "./pic/Monochrome." + EXTENTION;
    Var.quantizeColorSpace( MagickCore::GRAYColorspace ); Var.quantize( ); Var.write(("./pic/Grayscale." + EXTENTION)); Var = img; ImagePool[25].path = "./pic/Grayscale." + EXTENTION;
    
    // Equalize
    Var.equalize(); Var.write(("./pic/Equalize.") + EXTENTION); Var = img; ImagePool[26].path = "./pic/Equalize." + EXTENTION;
    
    // Shade
    Var.shade(30,30,true);  Var.write(("./pic/Shade_Grayscale." + EXTENTION)); Var = img; ImagePool[27].path = "./pic/Shade_Grayscale." + EXTENTION;
    Var.shade(30,30,false);  Var.write(("./pic/Shade_Color." + EXTENTION)); Var = img; ImagePool[28].path = "./pic/Shade_Color." + EXTENTION;
    
    // Sharpen
    Var.sharpen(0.0, 10.0); Var.write(("./pic/Sharpen.") + EXTENTION); Var = img; ImagePool[29].path = "./pic/Sharpen." + EXTENTION;
    
    // XOR
    Var.negate(); Var.write(("./pic/Xor." + EXTENTION)); Var = img; ImagePool[30].path = "./pic/Xor." + EXTENTION;

    // Bit-Planes
    std::string cmd = "./slice " + PathS;
    FILE* fp = popen(cmd.c_str(),"r");    
    system("sleep 1");
    ImagePool[31].path = "./pic/bit_plane0.png"; 
    ImagePool[32].path = "./pic/bit_plane1.png"; 
    ImagePool[33].path = "./pic/bit_plane2.png"; 
    ImagePool[34].path = "./pic/bit_plane3.png"; 
    ImagePool[35].path = "./pic/bit_plane4.png"; 
    ImagePool[36].path = "./pic/bit_plane5.png"; 
    ImagePool[37].path = "./pic/bit_plane6.png"; 
    ImagePool[38].path = "./pic/bit_plane7.png"; 

    // Difference
    if(COMPARE){
        fs::remove_all("./pic/Difference." + EXTENTION);
        char Comp [100] = "compare "; strcat(Comp,Path); strcat(Comp," "); strcat(Comp,PathComp); strcat(Comp," ./pic/Difference."); strcat(Comp,ext); system(Comp); ImagePool[39].path = "./pic/Difference." + EXTENTION;
    }


    // Grey bits
    // Not in Magick++
    // char gsLuminance [100] = "convert "; strcat(gsLuminance,Path); strcat(gsLuminance," -grayscale Rec709Luminance ./pic/Rec709Luminance."); strcat(gsLuminance,ext); system(gsLuminance);  ImagePool[20].path = "./pic/Rec709Luminance." + EXTENTION;
    // char gsLineargray [100] = "convert "; strcat(gsLineargray,Path); strcat(gsLineargray," -colorspace LinearGray ./pic/Linear_Gray."); strcat(gsLineargray,ext); system(gsLineargray); ImagePool[21].path = "./pic/Linear_Gray." + EXTENTION;
}

// Aoleu
static void glfw_error_callback(int error, const char* description){
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int argc, char** argv){

    // Parse arguments
    argparse::ArgumentParser program("StegoSaurus", "1.2", argparse::default_arguments::help);

    // $ -i INPUT
    // $ -i INPUT --compare C
    // $ -i INPUT --lsb-encode M
    // $ -i INPUT --lsb-decode
    program.add_argument("-i","--input")
        .help("| [REQUIRED] | Input image to analize.")
        .nargs(1);
    program.add_argument("--compare")
        .default_value(false)
        // .implicit_value(true)
        .nargs(1)
        .help("| [OPTIONAL] | Compare image file with a second given image.");
    program.add_argument("--lsb-encode")
        .default_value(false)
        .nargs(1)
        // .implicit_value(true)
        .help("| [OPTIONAL] | Encode input image with a given text using LSB encoding.");
    program.add_argument("--lsb-decode")
        .default_value(false)
        .implicit_value(true)
        .help("| [OPTIONAL] | Decode input image using LSB decoding");
    program.add_argument("--debug")
        .default_value(false)
        .implicit_value(true)
        .help("| [OPTIONAL] | Enable verbosity");
    program.add_description("Usage: stegosaurus -i file [OPTIONS]");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    // Verbosity
    if (program["--debug"] == true) DEBUG = true;

    // Input
    auto ImagePathS = program.get<std::string>("--input");
    auto isCompare = program.is_used("--compare");
    auto isEncodeLSB = program.is_used("--lsb-encode");
    std::string SecondImagePathS = "";
    std::string SecondImageExtentionS = "";
    std::string EncodeMessage = "";
    //auto SecondImagePathS = program.get<std::string>("--compare");s

    // Compare
    if (isCompare) {
        SecondImagePathS = program.get<std::string>("--compare");
    }
    std::string ImageExtentionS = GetExtention(ImagePathS);
        
    if(SecondImagePathS != ""){
        SecondImageExtentionS = GetExtention(SecondImagePathS);
        if(ImageExtentionS.compare(SecondImageExtentionS)!=0){
            std::cout<<"[ERROR]: Images extentions are not the same"<<endl;
            return 1;
        }
        COMPARE = true;
        POOL_SIZE++;
    }

    // Convert Path and Extention to char[]
    char ImagePathC [ImagePathS.length() + 1];
    strcpy(ImagePathC,ImagePathS.c_str());
    char ImageExtentionC [ImageExtentionS.length() + 1];
    strcpy(ImageExtentionC,ImageExtentionS.c_str());

    char SecondImagePathC [SecondImagePathS.length() + 1];
    strcpy(SecondImagePathC,SecondImagePathS.c_str());   

    // Encode LSB
    if (isEncodeLSB){
        // Get Message Arg
        EncodeMessage = program.get<std::string>("--lsb-encode");
        ENCODE_LSB = true;

        // Put Message onto file
        std::ofstream tmpFile("secret");
        tmpFile<<EncodeMessage;

        // Exec Encode
        std::string cmd = "./encode " + ImagePathS + " secret lsb_encoded.png";
        FILE* fp = popen(cmd.c_str(),"r");    
        std::cout<<"[LSB-ENCODE] Message Encoded into new file `lsb_encoded.png`";
        
        // Finish Execution
        return 1;
    }

    // Decode LSB
    if (program["--lsb-decode"] == true && COMPARE == false && DECODE_LSB == false){
        // Exec Decode
        DECODE_LSB = true;
        FILE *fp = popen ("./decode lsb_encoded.png","r");
        
        // Read Extracted Message
        system("sleep 1");
        std::string read; 
        std::ifstream Extract("StegoSaurus-LSB-Extract");
        std::cout<<"[LSB-DECODE]: ";
        while (getline (Extract, read)){ cout<<read; }
        Extract.close();

        // Finish Execution
        return 1;
    }

    // Argument Error
    if (COMPARE && ENCODE_LSB || COMPARE && DECODE_LSB){
        std::cout<<"[ERROR]: Invalid Arguments. Use `stegosaurs --help` for more info."<<endl;
        return 1;
    }

    // Intialize ImageMagick
    //InitializeMagick(*argv);
    Magick::Image img1, img2;
    img1.read(ImagePathS);  
    if(COMPARE) img2.read(SecondImagePathS);

    // Initialize Image Pool
    ImagePool[0].path = ImagePathS;

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 2;

    // TODO
    fs::remove_all("./pic");
    fs::create_directory("./pic");
    //system("mkdir ./pic");

    // GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Window creation
    GLFWwindow* window = glfwCreateWindow(1280, 720, "StegoSaurus", NULL, NULL);
    if (window == NULL)
        return 3;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Theme
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Flags
    bool show_demo_window = false;
    bool show_image_window = true;
    bool show_exif_window = true;
    bool show_about_window = false;
    bool show_lsb_encode_window = false;
    bool show_lsb_decode_window = false;
    bool switch_exif_text = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // FileBrowser Catalog
    ImGui::FileBrowser fileDialogDecode;
    fileDialogDecode.SetTitle("Select Image to Decode");
    fileDialogDecode.SetTypeFilters({ ".png" });
    
    // Image initialization
    int my_image_width = 0;
    int my_image_height = 0;
    GLuint current_texture = 0;
    //GLuint TexturePool[28] = {0};
    
    // Generate variants
    if(COMPARE) GenerateVariants(ImagePathS,ImagePathC,SecondImagePathC,ImageExtentionC,ImagePool,img1,img2);  
    else GenerateVariants(ImagePathS,ImagePathC,ImagePathC,ImageExtentionC,ImagePool,img1,img1);

    // Initial size of images
    float fX = 25.0f, fY = 25.0f;

    // Upload pictures to RAM
    for (int i=0; i<POOL_SIZE; i++){
        char path [ImagePool[i].path.length() + 1];
        strcpy(path,ImagePool[i].path.c_str());
        GLuint &tex = ImagePool[i].texture;  
        bool ret = LoadTextureFromFile(path, &tex, &my_image_width, &my_image_height);
        //ret = GenerateOpenGL3Texture(ImagePool[i], my_image_width, my_image_height);
        
        if(DEBUG)std::cout<<"[DEBUG_OPENGL]: CREATED IMAGE FROM "<<ImagePool[i].path<<" WITH TEXTURE"<<ImagePool[i].texture<<std::endl;
    }

    // Load the Logo as well
    char LogoPath[11] = "./logo.png";
    int logo_width = 0;
    int logo_height = 0;
    GLuint logo_texture = 0;
    bool retLogo = LoadTextureFromFile(LogoPath, &logo_texture, &logo_width, &logo_height);

    // Set first image as active
    current_texture = ImagePool[0].texture;
    unsigned int currImg = 0;

    // Out to file exif
    std::ofstream OutFile("EXIF");
    std::ofstream OutFileExt("EXIF_EXTENDED");
    ExifTool *et = new ExifTool();
    TagInfo *info = et->ImageInfo(ImagePathC,NULL,5);
    if (info){
        for (TagInfo *i=info; i; i=i->next){
            OutFile<<"["<<i->name<<"]"<<" = "<<i->value<<endl;
        }
        for (TagInfo *i=info; i; i=i->next){
            OutFileExt<<i->name<<" = "<<i->value<<endl;
            if(i->group[0]!=NULL) OutFileExt<<"  [group[0]] = "<<i->group[0]<<endl;
            if(i->group[1]!=NULL) OutFileExt<<"  [group[2]] = "<<i->group[1]<<endl;
            if(i->group[2]!=NULL) OutFileExt<<"  [group[1]] = "<<i->group[2]<<endl;
            if(i->name!=NULL) OutFileExt<<"  [name] = "<<i->name<<endl;      
            if(i->desc!=NULL) OutFileExt<<"  [desc] = "<<i->desc<<endl;      
            if(i->id!=NULL) OutFileExt<<"  [id] = "<<i->id<<endl;      
            if(i->value!=NULL) OutFileExt<<"  [value] = "<<i->value<<endl;      
            if(i->valueLen!=NULL) OutFileExt<<"  [valueLen] = "<<i->valueLen<<endl;      
            if(i->num!=NULL) OutFileExt<<"  [num] = "<<i->num<<endl;      
            OutFileExt<<"  [numLen] = "<<i->numLen<<endl;                         
            OutFileExt<<"  [copyNum] = "<<i->copyNum<<endl;                       
        }
        delete info;
    } else if (et->LastComplete() <= 0) {
        std::cerr<< "[ERROR]: At Exif Outfile Extraction"<<endl;
    }
    char *err = et->GetError();
    if (err) std::cerr<<err;
    delete et;

    // Get from file exif
    std::ifstream Exif("./EXIF", std::ios::in|std::ios::binary|std::ios::ate);
    std::ifstream ExifExtended("./EXIF_EXTENDED", std::ios::in|std::ios::binary|std::ios::ate);

//////////////////////////////////////////MAIN LOOP/////////////////////////////////////////////////////////
    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();

        // ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(ImGui::BeginMainMenuBar()){
            // if(ImGui::BeginMenu("File")){;
            //     if(ImGui::MenuItem("Open")){
            //         //fileDialog.Open();
            //     }
            //     ImGui::EndMenu();
            // }
            if(ImGui::BeginMenu("StegoSaurus")){
                if(ImGui::MenuItem("About")){ 
                    if(show_about_window == false) 
                        show_about_window = true;
                }
                if(ImGui::MenuItem("Quit")){
                    if(DEBUG)std::cout<<"[DEBUG_MENU]: NICE, EXIT BY MenuItem(\"Quit\")";
                    return -1;
                }
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("LSB")){
                if(ImGui::MenuItem("Encode")){
                    if(show_lsb_encode_window == false)
                        show_lsb_encode_window = true;
                }
                if(ImGui::MenuItem("Decode")){
                    fileDialogDecode.Open();
                    if(show_lsb_decode_window == false)
                        show_lsb_decode_window = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // 2. Main window
        {
            std::string SelectedImage;
            ImGui::Begin("Toolbox");                          

            ImGui::Checkbox("IMAGE WINDOW", &show_image_window);
           
            ImGui::SameLine();
            ImGui::Checkbox("EXIF WINDOW", &show_exif_window);
            ImGui::SameLine();
            ImGui::Checkbox("EXIF EXTENDED", &switch_exif_text);
            if(ImGui::Button("<")){
                if(currImg <= 0)currImg = POOL_SIZE-1;
                else currImg--;
                current_texture = ImagePool[currImg].texture;
                
                //debug
                if(DEBUG)std::cout<<ImagePool[0].path<<" "<<EXTENTION;
            }
            ImGui::SameLine();
            ImGui::Text("MODE SELECT");             
            ImGui::SameLine();
            if(ImGui::Button(">")){
                currImg++;
                if(currImg >= POOL_SIZE) {
                    currImg = 0;
                    current_texture = ImagePool[0].texture;
                }
                current_texture = ImagePool[currImg].texture;
            }
            

            ImGui::SliderFloat(" ", &fX, 1.0f, 250.0f, "Image Size = %.3f");
            fY = fX;
            //ImGui::SliderFloat("slider float (log)", &fY, -10.0f, 10.0f, "%.4f", ImGuiSliderFlags_Logarithmic);

            // Performance
            ImGui::Separator();
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }


        // 3. Image window.
        if (show_image_window){
            ImGui::Begin("Image", &show_image_window); 
            ImGui::Text("pointer = %p", current_texture);
            ImGui::Text("size = %d x %d", my_image_width, my_image_height);
            
            ImVec2 display_min = ImVec2(10.0f, 10.0f);
            ImVec2 display_size = ImVec2((my_image_width)/100 * fX, (my_image_height/100) * fY);
            ImVec2 texture_size = ImVec2(256.0f, 256.0f);
            ImVec2 uv0 = ImVec2(display_min.x / texture_size.x, display_min.y / texture_size.y);

            // Normalized coordinates of pixel (110,210) in a 256x256 texture.
            ImVec2 uv1 = ImVec2((display_min.x + display_size.x) / texture_size.x, (display_min.y + display_size.y) / texture_size.y);

            ImGui::Text("uv0 = (%f, %f)", uv0.x, uv0.y);
            ImGui::Text("uv1 = (%f, %f)", uv1.x, uv1.y);
            

            // Mode Label
            std::string imgName;
            if(currImg == 0){
                imgName = "Input File";
            } else{
                imgName = ImagePool[currImg].path;
                imgName = imgName.erase(0,6);
                imgName = imgName.substr(0, imgName.length() - (EXTENTION.length() + 1));
            }
            imgName = "MODE: " + imgName;

            char labelMode [imgName.length() + 1];
            strcpy(labelMode,imgName.c_str());
            ImGui::Text(labelMode);

            // Image
            ImGui::Image((void*)(intptr_t)current_texture, ImVec2(display_size.x, display_size.y));      
            ImGui::End();
        }

        // 4. Exif window.
        if (show_exif_window){
            ImGui::Begin("Exif Data", &show_exif_window);
            
            // THIS METHOD DOES NOT WORK
            //std::string t;
            // char *line;
            // while(getline(Exif,t)){
            //     //std::cout<<"[DEBUG_EXIF]:"<<t<<std::endl;
            //     line = new char[t.length()+1];
            //     strcpy(line,t.c_str());
            //     //std::cout<<"[DEBUG_EXIF]:"<<line<<std::endl;
            //     ImGui::Text(line);
            // } 


            // Exif
            // MEMORY SEEKING MAGICK 
            std::streampos size;
            char * memblock;
            size = Exif.tellg();
            memblock = new char [size];
            Exif.seekg (0, std::ios::beg);
            Exif.read (memblock, size);
            //debug
            //if(DEBUG)std::cout<<"[DEBUG_EXIF][MEMBLOCK]"<<std::endl<<memblock<<std::endl;
            if(!switch_exif_text)ImGui::Text(memblock);            
            // Haha, memory leak go BRRRRR
            delete[] memblock;

            // Exif Extended
            std::streampos size2;
            char * memblock2;
            size2 = ExifExtended.tellg();
            memblock2 = new char [size2];
            ExifExtended.seekg (0, std::ios::beg);
            ExifExtended.read (memblock2, size2);
            if(switch_exif_text)ImGui::Text(memblock2);            
            delete[] memblock2;

            ImGui::End();
        }

        // 5. About window
        if (show_about_window){
            ImGui::Begin("About", &show_about_window);
            ImGui::Image((void*)(intptr_t)logo_texture, ImVec2(logo_width*0.2f, logo_height*0.2f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
            ImGui::Text("StegoSaurus 1.2");
            ImGui::Text("Iulian Ionel Bocse, USH 2022");
            ImGui::Separator();
            ImGui::Text("C++  Magick++  OpenCV2  ImGui");
            ImGui::Text("     OpenGL3   GLFW3");
            ImGui::End();
        }

        // 6. LSB - Encode Window
        if (show_lsb_encode_window){
            ImGui::Begin("LSB - Encode", &show_lsb_encode_window);
            ImGui::Text("Type the message to encode into the image:");
            static char text[1024];
            static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
            ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 5), flags);
            if(ImGui::Button("Encode")){
                EncodeLSB(text, ImagePathS);
                // std::ifstream message("./secret");
                // if (isFileEmpty(message)) ImGui::OpenPopup("LSB - Encode: Error");
                ImGui::OpenPopup("LSB - Encode: Done");
                // system("rm secret");
            }

            // if (ImGui::BeginPopup("LSB - Encode: Error")){;
            //     ImGui::Text("Encoding Not Successfull!");
            //     ImGui::EndPopup();
            // }

            if (ImGui::BeginPopup("LSB - Encode: Done")){;
                ImGui::Text("Encoding Done!");
                ImGui::EndPopup();
            }
            ImGui::End();
        }

        // 7. LSB - Decode Window
        // FileDialog
        fileDialogDecode.Display();
        if(fileDialogDecode.HasSelected()){
            std::string SelectedImage;
            // fileDialogDecode.ClearSelected();
            SelectedImage = fileDialogDecode.GetSelected().string();
            DecodedMessage = DecodeLSB(SelectedImage);
            // DecodedMessage = "Acest mesaj a fost ascuns folosind metoda LSB";
        }

        if (show_lsb_decode_window){
            ImGui::Begin("LSB - Decode", &show_lsb_decode_window);
            ImGui::Text("The decoded message:");
            ImGui::Separator();

            char text[DecodedMessage.length() + 1];
            strcpy(text,DecodedMessage.c_str());
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), text);
            fileDialogDecode.ClearSelected();
            ImGui::End();
        }

                

        // Render
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(window);
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////


    // Mr Clean
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    Exif.close();
    // system("rm secret");
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
