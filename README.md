# usdosr

USDOS wrapped in Rcpp package for use in R.  Provides a single function `run_usdos` that takes a config file as input and runs the model, writing output to file.

The main advantage of this is that it allows easy parallelization at the level of config files using parallel packages in R.  The use of the `doRNG` package also ensures thread-safe random number generation and reproducible results.  

## Installation

To install, either download the repository and build the package yourself, or use 
```r
devtools::install_github("webblabb/usdosr")
```
Since the repository is private, you'll have to generate a personal access token in github and save it to the `GITHUB_PAT` environment variable.  For future use, you can add `GITHUB_PAT = "<token from github>"` to the `~/.Renviron` file.

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