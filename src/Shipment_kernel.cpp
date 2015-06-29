#include <Shipment_kernel.h>
#include <County.h>
#include <iostream>

Shipment_kernel::Shipment_kernel(double a, double b, std::string type, bool binning_on) :
    a(a), b(b), binning_on(binning_on)
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
    set_bins();
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

void Shipment_kernel::set_bin_size(double in_size)
{
    //Sets the size of each bin
    bin_size = in_size;
}

void Shipment_kernel::set_longest_distance(double in_dist)
{
    //Maximum distance to bin for.
    longest_distance = in_dist;
}

void Shipment_kernel::set_bins()
{
    bins.clear();
    int n_bins = int(longest_distance / bin_size) + 1;
    bins.reserve(n_bins);
    for(int i = 0; i < n_bins; i++)
    {
        bins.push_back(i*bin_size);
    }
}

double Shipment_kernel::get_bin(double d)
{
    //Binary search for the correct bin of d
    bool done = false;
    int lower = 0; //Lower index of current sub-vector
    int upper = bins.size() - 1; //Upper index of current sub-vector
    int mid = 0;
    int n_tries = 0;

    do
    {
        mid = lower + int(ceil((upper-lower) / 2)); //Get mid index of current sub-vector.
        if(d == bins[mid])
        {
            done = true;
            return bins[mid];
        }
        else if(d > bins[mid])
        {
            lower = mid;
        }
        else if(d < bins[mid])
        {
            upper = mid;
        }
        if(abs(upper - lower) == 1)
        {
            double udiff = std::abs(bins[upper] - d);
            double ldiff = std::abs(bins[lower] - d);
            if(ldiff < udiff)
            {
                done = true;
                return bins[lower];
            }
            else
            {
                done = true;
                return bins[upper];
            }
        }
        n_tries += 1;
        if(n_tries >= 100)
        {
            std::cout << "Stuck in when getting bin for distance " << d << ". Exiting..." << std::endl;
            exit(EXIT_FAILURE);
        }
    } while(!done);

    return -1;
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

    if(binning_on)
    {
        d = get_bin(d);
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

    if(binning_on)
    {
        d = get_bin(d);
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
