setwd("/Users/kimtsao/Desktop/PremModel/USDOSv2.1/inputfiles")

premises = read.delim("uk_edited.txt",header=F)
	colnames(premises) = c("ID","county","x","y","cattle","sheep") # x-y order reversed in FLAPS?
	
setwd("/Users/kimtsao/Desktop/PremModel/USDOSv2.1")

cells = read.delim("uk_nopower_cells.txt", header=T, colClasses = c(rep("numeric",5),"character"))
	colnames(cells) = c("ID","x","y","s","nfarms","fips")
		
drawCell <- function(cell, color="red") { # cell is row from cells
	xvec = c(cell["x"], cell["x"]+cell["s"], cell["x"]+cell["s"], cell["x"])
	yvec = c(cell["y"], cell["y"], cell["y"]+cell["s"], cell["y"]+cell["s"])
	polygon(xvec,yvec,border=color)
}

minx = min(cells$x)
maxx = max(cells$x+cells$s)
miny = min(cells$y)
maxy = max(cells$y+cells$s)

# plot some number of prems picked at intervals
pToPlot = seq(1,nrow(premises),length.out=1000) # equal intervals
	
# plot premises
dev.new(width=12,height=4.5,units="in")
par(mfrow=c(1,2))
plot(premises[pToPlot,"x"], premises[pToPlot,"y"], xaxt="n", yaxt="n", xlab="", ylab="", 
	xlim=c(minx,maxx), ylim=c(miny,maxy), bty="n", pch="*", col="gray")
# draw cells on top
apply(cells[,1:5],1,drawCell) #draw all cells

#  # label cells by id number
for (c in 1:nrow(cells)){
	xpt = cells[c,"x"]+cells[c,"s"]/2
	ypt = cells[c,"y"]+cells[c,"s"]/2
	show = paste(cells[c,"ID"])
	text(xpt,ypt,show)
}

plot(premises[pToPlot,"x"], premises[pToPlot,"y"], xaxt="n", yaxt="n", xlab="", ylab="", 
	xlim=c(minx,maxx), ylim=c(miny,maxy), bty="n", pch="*", col="gray")
# draw cells on top
apply(ucells,1,drawCell) #draw all cells
