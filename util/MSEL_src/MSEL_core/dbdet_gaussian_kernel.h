// This is brcv/seg/dbdet/edge/dbdet_gaussian_kernel.h
#ifndef dbdet_gaussian_kernel_h
#define dbdet_gaussian_kernel_h
//:
//\file
//\brief Gaussian derivative kernels
//\author Amir Tamrakar
//\date 09/09/06
//
//\verbatim
//  Modifications
//\endverbatim

#include "dbdet_kernel.h"
#include <vnl/vnl_math.h>

//: Gaussian derivative kernel base class
class dbdet_gaussian_kernel : public dbdet_kernel
{
public:
  int khs;                     //kernel half size
  vcl_vector<double> K_x, K_y; //separated kernels to minimize computation

protected:
  double sigma; ///< operator sigma

public:
  //: constructor given sigma and shifts
  dbdet_gaussian_kernel(double sigma_, double dx_=0.0, double dy_=0.0, double theta_=0.0): 
    dbdet_kernel((unsigned)(2*vcl_ceil(4*sigma_)+1), (unsigned)(2*vcl_ceil(4*sigma_)+1), dx_, dy_, theta_), 
    khs((int) vcl_ceil(4*sigma_)), K_x(2*khs+1, 0.0), K_y(2*khs+1, 0.0), sigma(sigma_)
  {
    compute_kernel();
  }
  //: destructor
  ~dbdet_gaussian_kernel(){}

  //: compute the kernel
  virtual void compute_kernel(bool /*separated_kernels_only*/=false){}

  //: recompute kernel with given subpixel shifts
  virtual void recompute_kernel(double dx_=0.0, double dy_=0.0, double theta_=0.0)
  {
    dx = dx_; 
    dy = dy_;
    theta = theta_;
    compute_kernel();
  }

};

//: Gaussian Left half kernel at the given orientation
//  Note: not separable
class dbdet_G_Lhalf_kernel : public dbdet_gaussian_kernel
{
public:
  dbdet_G_Lhalf_kernel(double sigma_, double dx_=0.0, double dy_=0.0, double theta_=0.0): dbdet_gaussian_kernel(sigma_, dx_, dy_, theta_){}
  ~dbdet_G_Lhalf_kernel(){}

  //: compute the kernel
  virtual void compute_kernel(bool /*separated_kernels_only*/=false)
  {   
    double ssq = sigma*sigma;
    //double pisig2 = 2*vnl_math::pi*ssq;
    double cc = vcl_sqrt(2*vnl_math::pi)*ssq*sigma;

    //not separable
    for (int x = -khs; x <= khs; x++){
      for (int y = -khs; y <= khs; y++){

        double xx = x* vcl_cos(theta) + y*vcl_sin(theta) - dx;
        double yy = x*-vcl_sin(theta) + y*vcl_cos(theta) - dy;

        //only one half is a Gaussian (the other half is zeros)
        if (yy>=0)
          top_left_[(x+khs)*istep_+ (y+khs)*jstep_] = 0.0;
        else
          //top_left_[(x+khs)*istep_+ (y+khs)*jstep_] = 2*vcl_exp(-xx*xx/(2*ssq))*vcl_exp(-yy*yy/(2*ssq))/pisig2;
          top_left_[(x+khs)*istep_+ (y+khs)*jstep_] = vcl_exp(-xx*xx/(2*ssq))*yy*-vcl_exp(-yy*yy/(2*ssq))/cc;
      }
    }
  }

};

//: Gaussian Right half kernel at the given orientation
//  Note: not separable
class dbdet_G_Rhalf_kernel : public dbdet_gaussian_kernel
{
public:
  dbdet_G_Rhalf_kernel(double sigma_, double dx_=0.0, double dy_=0.0, double theta_=0.0): dbdet_gaussian_kernel(sigma_, dx_, dy_, theta_){}
  ~dbdet_G_Rhalf_kernel(){}

  //: compute the kernel
  virtual void compute_kernel(bool /*separated_kernels_only*/=false)
  {   
    double ssq = sigma*sigma;
    //double pisig2 = 2*vnl_math::pi*ssq;
    double cc = vcl_sqrt(2*vnl_math::pi)*ssq*sigma;

    //not separable
    for (int x = -khs; x <= khs; x++){
      for (int y = -khs; y <= khs; y++){

        double xx = x* vcl_cos(theta) + y*vcl_sin(theta) - dx;
        double yy = x*-vcl_sin(theta) + y*vcl_cos(theta) - dy;

        //only one half is a Gaussian (the other half is zeros)
        if (yy<0)
          top_left_[(x+khs)*istep_+ (y+khs)*jstep_] = 0.0;
        else
          //top_left_[(x+khs)*istep_+ (y+khs)*jstep_] = 2*vcl_exp(-xx*xx/(2*ssq))*vcl_exp(-yy*yy/(2*ssq))/pisig2;
          top_left_[(x+khs)*istep_+ (y+khs)*jstep_] = vcl_exp(-xx*xx/(2*ssq))*yy*vcl_exp(-yy*yy/(2*ssq))/cc;
      }
    }
  }

};

//: simple Gaussian smoothing kernel
//  K_x = G_x, K_y = G_y
class dbdet_G_kernel : public dbdet_gaussian_kernel
{
public:
  dbdet_G_kernel(double sigma_, double dx_=0.0, double dy_=0.0): dbdet_gaussian_kernel(sigma_, dx_, dy_){}
  ~dbdet_G_kernel(){}

  //: compute the kernel
  virtual void compute_kernel(bool separated_kernels_only=false)
  {   
    double ssq = sigma*sigma;
    double sq2pisig = vcl_sqrt(2*vnl_math::pi)*sigma;
    
    //1-d kernels
    for(int x = -khs; x <= khs; x++)
      K_x[x+khs] = vcl_exp(-(x-dx)*(x-dx)/(2*ssq))/sq2pisig;
    for(int y = -khs; y <= khs; y++)
      K_y[y+khs] = vcl_exp(-(y-dy)*(y-dy)/(2*ssq))/sq2pisig;

    if (!separated_kernels_only){
      for (unsigned i=0; i<ni_; i++){
        for (unsigned j=0; j<nj_; j++){
          top_left_[i*istep_+j*jstep_] = K_x[i]*K_y[j];
        }
      }
    }
  }

};

//: Gx kernel
//  K_x = dG_x, K_y = G_y
class dbdet_Gx_kernel : public dbdet_gaussian_kernel
{
public:
  dbdet_Gx_kernel(double sigma_, double dx_=0.0, double dy_=0.0): dbdet_gaussian_kernel(sigma_, dx_, dy_){}
  ~dbdet_Gx_kernel(){}

  //: compute the kernel
  virtual void compute_kernel(bool separated_kernels_only=false)
  {
    double ssq = sigma*sigma;
    double sq2pisig = vcl_sqrt(2*vnl_math::pi)*sigma;
    
    //1-d kernels
    for(int x = -khs; x <= khs; x++)
      K_x[x+khs] = -(x-dx)*vcl_exp(-(x-dx)*(x-dx)/(2*ssq))/(sq2pisig*ssq);
    for(int y = -khs; y <= khs; y++)
      K_y[y+khs] = vcl_exp(-(y-dy)*(y-dy)/(2*ssq))/sq2pisig;

    if (!separated_kernels_only){
      for (unsigned i=0; i<ni_; i++){
        for (unsigned j=0; j<nj_; j++){
          top_left_[i*istep_+j*jstep_] = K_x[i]*K_y[j];
        }
      }
    }
  }
};

//: Gy kernel
//  K_x = G_x, K_y = dG_y
class dbdet_Gy_kernel : public dbdet_gaussian_kernel
{
public:
  dbdet_Gy_kernel(double sigma_, double dx_=0.0, double dy_=0.0): dbdet_gaussian_kernel(sigma_, dx_, dy_){}
  ~dbdet_Gy_kernel(){}
  
  //: compute the kernel
  virtual void compute_kernel(bool separated_kernels_only=false)
  {
    double ssq = sigma*sigma;
    double sq2pisig = vcl_sqrt(2*vnl_math::pi)*sigma;
    
    //1-d kernels
    for(int x = -khs; x <= khs; x++)
      K_x[x+khs] = vcl_exp(-(x-dx)*(x-dx)/(2*ssq))/sq2pisig;
    for(int y = -khs; y <= khs; y++)
      K_y[y+khs] = -(y-dy)*vcl_exp(-(y-dy)*(y-dy)/(2*ssq))/(sq2pisig*ssq);

    if (!separated_kernels_only){
      for (unsigned i=0; i<ni_; i++){
        for (unsigned j=0; j<nj_; j++){
          top_left_[i*istep_+j*jstep_] = K_x[i]*K_y[j];
        }
      }
    }
  }
};

//: Gxx kernel
//  K_x = d2G_x, K_y = G_y
class dbdet_Gxx_kernel : public dbdet_gaussian_kernel
{
public:
  dbdet_Gxx_kernel(double sigma_, double dx_=0.0, double dy_=0.0): dbdet_gaussian_kernel(sigma_, dx_, dy_){}
  ~dbdet_Gxx_kernel(){}

  //: compute the kernel
  virtual void compute_kernel(bool separated_kernels_only=false)
  {
    double ssq = sigma*sigma;
    double sq2pisig = vcl_sqrt(2*vnl_math::pi)*sigma;
    
    //1-d kernels
    for(int x = -khs; x <= khs; x++)
      K_x[x+khs] = ((x-dx)*(x-dx)-ssq)*vcl_exp(-(x-dx)*(x-dx)/(2*ssq))/(sq2pisig*ssq*ssq);
    for(int y = -khs; y <= khs; y++)
      K_y[y+khs] = vcl_exp(-(y-dy)*(y-dy)/(2*ssq))/sq2pisig;

    if (!separated_kernels_only){
      for (unsigned i=0; i<ni_; i++){
        for (unsigned j=0; j<nj_; j++){
          top_left_[i*istep_+j*jstep_] = K_x[i]*K_y[j];
        }
      }
    }
  }
};

//: Gxy kernel
//  K_x = dG_x, K_y = dG_y
class dbdet_Gxy_kernel : public dbdet_gaussian_kernel
{
public:
  dbdet_Gxy_kernel(double sigma_, double dx_=0.0, double dy_=0.0): dbdet_gaussian_kernel(sigma_, dx_, dy_){}
  ~dbdet_Gxy_kernel(){}

  //: compute the kernel
  virtual void compute_kernel(bool separated_kernels_only=false)
  {
    double ssq = sigma*sigma;
    double sq2pisig = vcl_sqrt(2*vnl_math::pi)*sigma;
    
    //1-d kernels
    for(int x = -khs; x <= khs; x++)
      K_x[x+khs] = -(x-dx)*vcl_exp(-(x-dx)*(x-dx)/(2*ssq))/(sq2pisig*ssq);
    for(int y = -khs; y <= khs; y++)
      K_y[y+khs] = -(y-dy)*vcl_exp(-(y-dy)*(y-dy)/(2*ssq))/(sq2pisig*ssq);

    if (!separated_kernels_only){
      for (unsigned i=0; i<ni_; i++){
        for (unsigned j=0; j<nj_; j++){
          top_left_[i*istep_+j*jstep_] = K_x[i]*K_y[j];
        }
      }
    }
  }
};

//: Gyy kernel
//  K_x = G_x, K_y = d2G_y
class dbdet_Gyy_kernel : public dbdet_gaussian_kernel
{
public:
  dbdet_Gyy_kernel(double sigma_, double dx_=0.0, double dy_=0.0): dbdet_gaussian_kernel(sigma_, dx_, dy_){}
  ~dbdet_Gyy_kernel(){}
  
  //: compute the kernel
  virtual void compute_kernel(bool separated_kernels_only=false)
  {
    double ssq = sigma*sigma;
    double sq2pisig = vcl_sqrt(2*vnl_math::pi)*sigma;
    
    //1-d kernels
    for(int x = -khs; x <= khs; x++)
      K_x[x+khs] = vcl_exp(-(x-dx)*(x-dx)/(2*ssq))/sq2pisig;
    for(int y = -khs; y <= khs; y++)
      K_y[y+khs] = ((y-dy)*(y-dy)-ssq)*vcl_exp(-(y-dy)*(y-dy)/(2*ssq))/(sq2pisig*ssq*ssq);

    if (!separated_kernels_only){
      for (unsigned i=0; i<ni_; i++){
        for (unsigned j=0; j<nj_; j++){
          top_left_[i*istep_+j*jstep_] = K_x[i]*K_y[j];
        }
      }
    }
  }
};

//: Gxxx kernel
//  K_x = d3G_x, K_y = G_y
class dbdet_Gxxx_kernel : public dbdet_gaussian_kernel
{
public:
  dbdet_Gxxx_kernel(double sigma_, double dx_=0.0, double dy_=0.0): dbdet_gaussian_kernel(sigma_, dx_, dy_){}
  ~dbdet_Gxxx_kernel(){}
  
  //: compute the kernel
  virtual void compute_kernel(bool separated_kernels_only=false)
  {
    double ssq = sigma*sigma;
    double sq2pisig = vcl_sqrt(2*vnl_math::pi)*sigma;
    
    //1-d kernels
    for(int x = -khs; x <= khs; x++)
      K_x[x+khs] = (x-dx)*(3*ssq -(x-dx)*(x-dx))*vcl_exp(-(x-dx)*(x-dx)/(2*ssq))/(sq2pisig*ssq*ssq*ssq);
    for(int y = -khs; y <= khs; y++)
      K_y[y+khs] = vcl_exp(-(y-dy)*(y-dy)/(2*ssq))/sq2pisig;

    if (!separated_kernels_only){
      for (unsigned i=0; i<ni_; i++){
        for (unsigned j=0; j<nj_; j++){
          top_left_[i*istep_+j*jstep_] = K_x[i]*K_y[j];
        }
      }
    }
  }
};

//: Gxxy kernel
//  K_x = d2G_x, K_y = dG_y
class dbdet_Gxxy_kernel : public dbdet_gaussian_kernel
{
public:
  dbdet_Gxxy_kernel(double sigma_, double dx_=0.0, double dy_=0.0): dbdet_gaussian_kernel(sigma_, dx_, dy_){}
  ~dbdet_Gxxy_kernel(){}
  
  //: compute the kernel
  virtual void compute_kernel(bool separated_kernels_only=false)
  {
    double ssq = sigma*sigma;
    double sq2pisig = vcl_sqrt(2*vnl_math::pi)*sigma;
    
    //1-d kernels
    for(int x = -khs; x <= khs; x++)
      K_x[x+khs] = ((x-dx)*(x-dx)-ssq)*vcl_exp(-(x-dx)*(x-dx)/(2*ssq))/(sq2pisig*ssq*ssq);
    for(int y = -khs; y <= khs; y++)
      K_y[y+khs] = -(y-dy)*vcl_exp(-(y-dy)*(y-dy)/(2*ssq))/(sq2pisig*sigma*ssq);

    if (!separated_kernels_only){
      for (unsigned i=0; i<ni_; i++){
        for (unsigned j=0; j<nj_; j++){
          top_left_[i*istep_+j*jstep_] = K_x[i]*K_y[j];
        }
      }
    }
  }
};

//: Gxyy kernel
//  K_x = dG_x, K_y = d2G_y
class dbdet_Gxyy_kernel : public dbdet_gaussian_kernel
{
public:
  dbdet_Gxyy_kernel(double sigma_, double dx_=0.0, double dy_=0.0): dbdet_gaussian_kernel(sigma_, dx_, dy_){}
  ~dbdet_Gxyy_kernel(){}
  
  //: compute the kernel
  virtual void compute_kernel(bool separated_kernels_only=false)
  {
    double ssq = sigma*sigma;
    double sq2pisig = vcl_sqrt(2*vnl_math::pi)*sigma;
    
    //1-d kernels
    for(int x = -khs; x <= khs; x++)
      K_x[x+khs] = -(x-dx)*vcl_exp(-(x-dx)*(x-dx)/(2*ssq))/(sq2pisig*ssq);
    for(int y = -khs; y <= khs; y++)
      K_y[y+khs] = ((y-dy)*(y-dy)-ssq)*vcl_exp(-(y-dy)*(y-dy)/(2*ssq))/(sq2pisig*ssq*ssq);

    if (!separated_kernels_only){
      for (unsigned i=0; i<ni_; i++){
        for (unsigned j=0; j<nj_; j++){
          top_left_[i*istep_+j*jstep_] = K_x[i]*K_y[j];
        }
      }
    }
  }
};

//: Gyyy kernel
//  K_x = G_x, K_y = d3G_y
class dbdet_Gyyy_kernel : public dbdet_gaussian_kernel
{
public:
  dbdet_Gyyy_kernel(double sigma_, double dx_=0.0, double dy_=0.0): dbdet_gaussian_kernel(sigma_, dx_, dy_){}
  ~dbdet_Gyyy_kernel(){}
  
  //: compute the kernel
  virtual void compute_kernel(bool separated_kernels_only=false)
  {
    double ssq = sigma*sigma;
    double sq2pisig = vcl_sqrt(2*vnl_math::pi)*sigma;
    
    //1-d kernels
    for(int x = -khs; x <= khs; x++)
      K_x[x+khs] = vcl_exp(-(x-dx)*(x-dx)/(2*ssq))/sq2pisig;
    for(int y = -khs; y <= khs; y++)
      K_y[y+khs] = (y-dy)*(3*ssq -(y-dy)*(y-dy))*vcl_exp(-(y-dy)*(y-dy)/(2*ssq))/(sq2pisig*ssq*ssq*ssq);

    if (!separated_kernels_only){
      for (unsigned i=0; i<ni_; i++){
        for (unsigned j=0; j<nj_; j++){
          top_left_[i*istep_+j*jstep_] = K_x[i]*K_y[j];
        }
      }
    }
  }
};

#endif // dbdet_gaussian_kernel_h
