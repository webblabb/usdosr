// This file was generated by Rcpp::compileAttributes
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

// run_usdos
int run_usdos(std::string cfile);
RcppExport SEXP usdosr_run_usdos(SEXP cfileSEXP) {
BEGIN_RCPP
    Rcpp::RObject __result;
    Rcpp::RNGScope __rngScope;
    Rcpp::traits::input_parameter< std::string >::type cfile(cfileSEXP);
    __result = Rcpp::wrap(run_usdos(cfile));
    return __result;
END_RCPP
}
