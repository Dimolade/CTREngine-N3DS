#include "CTR/AutoIncludes.h"
using namespace std;

class RenderTest : public CTREntry {

CTRCamera* MainCam;
CTRImage* catbruh;
float hue = 0.0f ;
public:
inline void EntryPoint() override  {
MainCam =  Scene::ConstCTRCamera(0 ) ;
MainCam->RenderSpace =  "Render" ;
MainCam->Scissoring =  CTRScissor(3 , Vector2(0 , 0 ), Vector2(200 , 120 )) ;
Scene::AddCamera(MainCam );
catbruh =  Scene::ConstCTRImage("Catbruh" , "RenderSpace" , "catbruh" ) ;
catbruh->position =  Vector3(200 , 120 , 0 ) ;
Scene::AddSceneObject(catbruh );
CTRSound* music = Scene::ConstCTRSound("Music" , "Sounds" , "NewWelcomeMat" ) ;
Scene::AddSceneObject(music );
music->Play (  ) ;
}public:
inline void OnFrame() override  {
hue +=  1.0f ;
if (hue >= 360.0f )  {
hue -=  360.0f ;
}
float r;
float g;
float b;
HSVtoRGB(hue , 1.0f , 1.0f , &r , &g , &b );
catbruh->color =  Color ( r * 255 , g * 255 , b * 255 , 255 , 128  ) ;
}inline void HSVtoRGB( float h, float s, float v, float* outR, float* outG, float* outB) {
float c = v * s ;
float x = c * ( 1-fabsf(fmodf(h / 60.0f,2)-1)  ) ;
float m = v - c ;
float r;
float g;
float b;
if (h < 60 )  {
r =  c ;
g =  x ;
b =  0 ;
}
else if (h < 120 )  {
r =  x ;
g =  c ;
b =  0 ;
}
else if (h < 180 )  {
r =  0 ;
g =  c ;
b =  x ;
}
else if (h < 240 )  {
r =  0 ;
g =  x ;
b =  c ;
}
else if (h < 300 )  {
r =  x ;
g =  0 ;
b =  c ;
}
else  {
r =  c ;
g =  0 ;
b =  x ;
}float R = r + m ;
float G = g + m ;
float B = b + m ;
outR =  &R ;
outG =  &G ;
outB =  &B ;
}
public:
RenderTest() {}
};

