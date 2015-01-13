# Projects latitude/longitude from FLAPS output via Albers Equal Area and
# formats data for input into USDOSv2.1

infile_vec = c("FLAPS_beef_results_20141217.csv", # names of FLAPS output files - one of each commodity type
	"12012015_101251__dairy.csv")
outfile = "FLAPS_beefdairy_20151201_formatted.txt" # output should be tab-delimited for USDOS

setwd("/Users/kimtsao/Desktop/PremModel/grid-test") # location of FLAPS output files
allprems = NULL
premcount = 0 # running total number of premises
ctypes = NULL
for (i in 1:length(infile_vec)){
  firstrow = 1+premcount # first row number for this commodity type
  in_data <- read.csv(infile_vec[i], 
	row.names=NULL,
	colClasses=c("character", #fips
	"character", #state (two letter abbrev)
	"numeric", #lat
	"numeric", #long
	"numeric", #population
	"character") #species/production type
	) 
  premcount = premcount+nrow(in_data) # running total number of premises
  ids = firstrow:premcount # premises ID numbers to add
  type = paste("commodityType",i,sep="")
  colnames(in_data) = c("fips","state","latitude","longitude","population",type)
  	
  # put each commodity type (linked to i) population in a different column
  itype = unique(in_data[,type])
  ctypes = c(ctypes,itype)
  prevTypes = matrix(0,nrow=nrow(in_data),ncol=i-1) # all other commodity types before this (1 to i-1) are 0s
  postTypes = matrix(0,nrow=nrow(in_data),ncol=length(infile_vec)-i) # all other commodity types after this (i+1 to length(infile_vec)) are 0s
  commodityCounts = cbind(prevTypes,
  	in_data[,"population"],
  	postTypes) 
  head(commodityCounts)
  
  towrite = cbind(ids, in_data[,"fips"], in_data[,"longitude"], in_data[,"latitude"], 
  	commodityCounts)
  allprems = rbind(allprems,towrite)
}
	colnames(allprems) = c("id","fips","longitude","latitude",ctypes)

# plot 500 random premises just to see
library(maps)
map('state')
rand_sample = sample(allprems[,"id"],500)
sample_x = allprems$longitude[rand_sample]
sample_y = allprems$latitude[rand_sample]
points(sample_x,sample_y)

# project lat/long via Albers Equal Area projection (output is based on unit sphere = 1)
library(mapproj)
proj_coords = mapproject(x=as.numeric(allprems[,"longitude"]), 
	y=as.numeric(allprems[,"latitude"]), projection="albers",
	parameters = c(29.5,45.5)) # parameters are parallel latitudes to center projection on
radius = 6371000 # radius of the earth in meters
scaled_xcoords = proj_coords$x*radius
scaled_ycoords = proj_coords$y*radius

range(scaled_xcoords)
range(scaled_ycoords)

allprems[,"latitude"] = scaled_ycoords
allprems[,"longitude"] = scaled_xcoords
	
write(t(allprems),outfile,sep="\t",ncolumns=ncol(allprems))
