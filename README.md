# usdosr

USDOS wrapped in Rcpp package for use in R.  Provides a single function `run_usdos` that takes a config file as input and runs the model, writing output to file.

## Installation

To install, either download the repository and build the package yourself, or use
```r
devtools::install_github("webblabb/usdosr")
```

## Use

An example use case would be to put all of the config files in a subdirectory of the working directory (`/configs`) and do a parallel loop over them:

```r
library(doParallel)
library(foreach)
library(iterators)
library(doRNG)

library(usdosr)

# Make and register cluster
cl <- makeCluster(3)
registerDoParallel(cl)

# Set seed for reproducible results
set.seed(1234)

# List all config files in working directory
configs <- list.files("configs")

out <- foreach(file = iter(configs), .errorhandling = "stop",
        .packages = c("usdosr")) %dorng% {

  run_usdos(paste("configs/", file, sep = ""))

}
```

Note that the `out` object will just contain zeros, but should have the random number seeds of each run stored as attributes that can be used to reproduce that specific run if necessary.

## Documentation

See the project [website](https://webblabb.github.io/usammusdos) for project documentation and manuals.
