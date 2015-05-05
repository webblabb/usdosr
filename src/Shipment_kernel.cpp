#include <Shipment_kernel.h>
#include <County.h>
#include <iostream>

Shipment_kernel::Shipment_kernel(double a, double b, std::string type) :
    a(a), b(b)
{
    if(type == "linear")
    {
        k_function = &Shipment_kernel::linear_distance_kernel;
        d_function = &Shipment_kernel::linear_euclidean;
    }
    else if(type == "quadratic")
    {
        k_function = &Shipment_kernel::quadratic_distance_kernel;
        d_function = &Shipment_kernel::quadratic_euclidean;
    }
    else
    {
        std::cout <<"Error: Unknown shipment kernel type: " << type << ". Exiting..."
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    a_sq = a*a;
    b_half = b * 0.5;
}

Shipment_kernel::~Shipment_kernel() {}

double Shipment_kernel::kernel(County* c1, County* c2)
{
    double d = (this->*d_function)(c1, c2);
    return (this->*k_function)(d);
}

double Shipment_kernel::distance(County* c1, County* c2, std::string type)
{
    if(type == "linear")
    {
        return linear_euclidean(c1, c2);
    }

    else if(type == "quadratic")
    {
        return quadratic_euclidean(c1, c2);
    }
    else
    {
        return (this->*d_function)(c1, c2);
    }

}

double Shipment_kernel::linear_distance_kernel(double d)
{
    return exp(-pow(d/a,b));
}

double Shipment_kernel::quadratic_distance_kernel(double sq_d)
{
    return exp(-pow(sq_d / a_sq, b_half));
}

double Shipment_kernel::linear_euclidean(County* c1, County* c2)
{
    double d;
    if(c1 == c2)
    {
        d = sqrt(c1->get_area()) * 0.5214;
    }
    else
    {
        const Point* p1 = c1->get_centroid();
        const Point* p2 = c2->get_centroid();
        d = sqrt((p1->x - p2->x) * (p1->x - p2->x) +
                (p1->y - p2->y) * (p1->y - p2->y));
    }
    return d;
}

double Shipment_kernel::quadratic_euclidean(County* c1, County* c2)
{
    double d;
    if(c1 == c2)
    {
        d = c1->get_area() * 0.5214 * 0.5214;
    }
    else
    {
        const Point* p1 = c1->get_centroid();
        const Point* p2 = c2->get_centroid();
        d = ((p1->x - p2->x) * (p1->x - p2->x) +
            (p1->y - p2->y) * (p1->y - p2->y));
    }
    return d;
}

void Shipment_kernel::test(double d)
{
    double d_sq = d*d;
    std::cout << "Result for \"" << d << "\" in the linear case:\t"
              << linear_distance_kernel(d) << std::endl;
    std::cout << "Result for \"" << d << "\" in the quadratic case:\t"
              << quadratic_distance_kernel(d_sq) << std::endl;
}
