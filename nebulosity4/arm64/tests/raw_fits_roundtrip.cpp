#include <libraw/libraw.h>
#include <fitsio.h>
#include <vector>
#include <cstdio>
int main(int argc,char**argv){
 if(argc!=3)return 1;LibRaw raw;
 if(raw.open_file(argv[1]) || raw.unpack() || raw.raw2image())return 2;
 fitsfile* file=nullptr;int status=0,naxis=0;long axes[3]={0,0,0};
 fits_open_image(&file,argv[2],READONLY,&status);fits_get_img_dim(file,&naxis,&status);fits_get_img_size(file,3,axes,&status);
 const long w=raw.imgdata.sizes.iwidth,h=raw.imgdata.sizes.iheight;
 if(status || axes[0]!=w || axes[1]!=h)return 3;
 std::vector<float> pixels(w*h);int nulls=0;
 fits_read_img(file,TFLOAT,1,w*h,nullptr,pixels.data(),&nulls,&status);fits_close_file(file,&status);if(status)return 4;
 long mismatch=0,flippedMismatch=0;
 for(long y=0;y<h;++y)for(long x=0;x<w;++x){
  float value=raw.imgdata.image[y*w+x][raw.FC(y,x)];
  mismatch+=pixels[y*w+x]!=value;flippedMismatch+=pixels[(h-1-y)*w+x]!=value;
 }
 printf("Sony ARW -> OpenNebulosity -> FITS: %ld x %ld, %ld sensor pixels\nDirect-order mismatches: %ld\nVertical-flip mismatches: %ld\n",w,h,w*h,mismatch,flippedMismatch);
 return mismatch==0 || flippedMismatch==0 ? 0:5;
}
