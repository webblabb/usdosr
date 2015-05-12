# Formats data for input into USDOSv2.1

# Assumes each premises has only one type (i.e. beef or dairy) with equal runs of each
# Input files should have column names, including "population","latitude","longitude", "runNumber"
# Combines one of each type into individual run files
# Adds a premises ID as first column
# Projects latitude/longitude from FLAPS output via Albers Equal Area
# Records metadata re: file pairings, demographic summary stats

library(data.table)
# to project lat/long via Albers Equal Area projection (output is based on unit sphere = 1)
library(mapproj)
projectAlbers <- function(in_x,in_y){
  proj_coords = mapproject(x=as.numeric(in_x), y=as.numeric(in_y), projection="albers",
                           parameters = c(29.5,45.5)) # parameters are parallel latitudes to center projection on
  radius = 6371000 # radius of the earth in meters
  scaled_xcoords = proj_coords$x*radius
  scaled_ycoords = proj_coords$y*radius
  
  return (cbind(scaled_xcoords,scaled_ycoords))
}

skewAndKurt <- function(values){
  n = length(values)
  mean_n = mean(as.numeric(values))
  diffs = as.numeric(values)-mean_n
  
  numerator = (1/n)*sum(diffs^3)
  denominator = ((1/(n-1))*sum(diffs^2))^(3/2)
  skew = numerator/denominator
  
  num2 = (1/n)*sum(diffs^4)
  dem2 = ((1/n)*sum(diffs^2))^2
  kurt = (num2/dem2) - 3
  
  output = c(skew, kurt)
  return (output)
}

#memory.limit(807200)
# location of FLAPS files
path = "c:/users/public/github/grid-test/FLAPS_runs"

typenames = c("beef","dairy")
typePaths = c(paste(path,"/beef2012",sep=""),
              paste(path,"/dairy2012",sep=""))

fm = lapply(typePaths, list.files)
fileMatches=matrix("",nrow=length(unlist(fm[1])),ncol=length(fm))
for (col in 1:length(fm)){
  fileMatches[,col] = unlist(fm[col])
}

# specify which replicates are associated with which files
runsPerFile = 100
repRanges=matrix(c(1,runsPerFile), nrow=1,ncol=2,byrow=T)
for (r in 1:9){ # 9 more sets,each increased by 100
  repRanges = rbind(repRanges, repRanges[r,]+runsPerFile)
}
fileMatches = cbind(formatC(repRanges,width=4,format="d",flag="0"),fileMatches)

setwd(path)
write(c("First_rep","Last_rep","Beef_file","Dairy_file"),"metadata2012.txt",sep="\t",ncolumns=ncol(fileMatches),append=F)
write(t(fileMatches),"metadata2012.txt",sep="\t",ncolumns=ncol(fileMatches),append=T)

write(c("Rep","Run", "SetStartRep","Type","Min","Max","Mean","Variance","Skew","Kurtosis"),
            "metadata2012.txt",sep="\t",ncolumns=length(writeStats),append=T)

ntypes = length(typePaths) # number of commodity types

for (m in 1:nrow(fileMatches)){ # one file set at a time
  startRep = as.numeric(fileMatches[m,1]) # where replicate numbers start for this file set
  rep = startRep # increased and reset within each file
  infile_vec = fileMatches[m,3:ncol(fileMatches)] # files to be combined
  repPremCount = rep(0,runsPerFile) # keep track of last premID for each of the replicates in this file set

  for (i in 1:ntypes){ # i is each type/file to be combined
    rep = startRep
    # set working directory to files for this commodity type
    setwd(typePaths[i])
    
    print ("Loading data file...")
    in_data <- fread(infile_vec[i], sep=',', skip=0, header=TRUE, colClasses="character")

# remove unneeded columns
    setkey(in_data, runNumber)
    setwd(path)
	
   startrun = 1 # 1st row for this run number
    for (run in 1:runsPerFile){
      print(paste("Processing run ",run," of ",typenames[i]," for set ",m))
      runSub = in_data[runNumber==startrun]   
      nInRun = nrow(runSub)
      # determine premises IDs for this run
      ids = (1+repPremCount[rep]):(nInRun+repPremCount[rep]) #premises ids, starting at last #
      # project x and y
      xandy = projectAlbers(runSub[,longitude],runSub[,latitude])
      # fill in zeros for all other population types
      allPops = matrix(0, nrow = nInRun, ncol=ntypes)
      # add population of this type
      pop = as.numeric(runSub[,population])
      allPops[,i] = pop
    
      # write to file
      towrite = cbind(
        ids,
        runSub[,fips],
        xandy,
        allPops
        )
    
      repChar = formatC(rep,width=4,format="d",flag="0")
      outfile = paste("flaps_",repChar,".txt",sep="")
    
      write(t(towrite),outfile,ncolumns=ncol(towrite),sep="\t")
    
      # calculate summary stats
      sum_stats = c(
        min(pop), #min
        max(pop), #max
        mean(pop),#mean
        var(pop), #variance
        skewAndKurt(pop) #skewness & kurtosis
      )
    
      writeStats = c(
        rep,
        run,
        formatC(startRep,width=4,format="d",flag="0"),
        typenames[i], # i.e. beef or dairy
        round(sum_stats, digits=2)
        )
      

      write(writeStats,"metadata2012.txt",sep="\t",ncolumns=length(writeStats),append=T)
    
      # prepare for next run
      startrun = startrun+1
      repPremCount[rep] = tail(ids,1) # set to last id value
      rep = rep+1
    }
    
} # end for each type
	
} # end for each file set



# plot random premises just to see
rand_sample = sample(1:nrow(xandy),1000)
sample_x = xandy[rand_sample,1]
sample_y = xandy[rand_sample,2]
points(sample_x,sample_y)
plot(sample_y~sample_x)