#ifndef KERNEL_F_H
#define KERNEL_F_H

#include <vector>
#include <string>
#include <iostream>

//A kernel function for shipments. Construct with parameters a and b
//as well as a string describing the type, either "linear" or
//"quadratic". Linear uses a standard kernel that is a function of
//distance, d. The quadratic alternative uses a kernel that is a
//function of d^2 to avoid square root operations. Both give
//identical results. To get a kernel value for a pair of counties
//call the function kernel(c1, c2), where the arguments are pointers to
//two counties. If both pointers point to the same county object
//the kernel value that is returned is for d = the average distance
//within a square of the same area as the county. The functions for
//getting the distance between two counties are built into the class.
//Also, for convenience, the function distance(c1, c2, type) can
//be used to get the distance between two counties. The type argument
//defaults to "linear" so if the actual euclidean distance is desired
//type can be omitted.

class County;

class Shipment_kernel
{
typedef double (Shipment_kernel::*k_fun_ptr)(double); //Kernel function pointer
typedef double (Shipment_kernel::*d_fun_ptr)(County*, County*); //Distance function pointer

public:
    Shipment_kernel(double a, double b, std::string type = "linear", bool binning_on = false);
    ~Shipment_kernel();
    double kernel(County* c1, County* c2);
    double distance(County* c1, County* c2, std::string type = "linear");
    void set_bin_size(double in_size);
    void set_longest_distance(double in_dist);
    void test(double d);

private:
    double a, b;
    bool binning_on;
    double a_sq, b_half;
    double bin_size = 20000;
    double longest_distance = 6000000;
    std::vector<double> bins;
    k_fun_ptr k_function;
    d_fun_ptr d_function;

    void set_bins();
    double get_bin(double d);
    double linear_distance_kernel(double d);
    double quadratic_distance_kernel(double sq_d);
    double linear_euclidean(County* c1, County* c2);
    double quadratic_euclidean(County* c1, County* c2);

};
#endif // KERNEL_F_H
