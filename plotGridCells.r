setwd("/Users/kimtsao/Desktop/PremModel/grid-test")

#premises = read.delim("heterotest.txt",header=F)
#	colnames(premises) = c("ID","x","y","pop") # x-y order reversed in FLAPS?

premises = read.delim("USprems.txt",header=F)
	colnames(premises) = c("ID","y","x","pop") # x-y order reversed in FLAPS?
	
cells = read.delim("2000f_731c_USprems.txt",header=F)
	colnames(cells) = c("ID","x","y","s","farms")
	
ucells = read.delim("75kmunif_1412c_USprems.txt",header=F)
	colnames(ucells) = c("ID","x","y","s","farms")
	
drawCell <- function(cell, color="red") { # cell is row from cells
	xvec = c(cell["x"], cell["x"]+cell["s"], cell["x"]+cell["s"], cell["x"])
	yvec = c(cell["y"], cell["y"], cell["y"]+cell["s"], cell["y"]+cell["s"])
	polygon(xvec,yvec,border=color)
}

minx = min(cells$x)
maxx = max(cells$x+cells$s)
miny = min(cells$y)
maxy = max(cells$y+cells$s)

# uminx = min(ucells$x)
# umaxx = max(ucells$x+ucells$s)
# uminy = min(ucells$y)
# umaxy = max(ucells$y+ucells$s)

# plot 1000 prems picked at intervals
pToPlot = seq(1,nrow(premises),length.out=5000) # equal intervals
	
# plot premises
dev.new(width=12,height=4.5,units="in")
par(mfrow=c(1,2))
plot(premises[pToPlot,"x"], premises[pToPlot,"y"], xaxt="n", yaxt="n", xlab="", ylab="", 
	xlim=c(minx,maxx), ylim=c(miny,maxy), bty="n", pch="*", col="gray")
# draw cells on top
apply(cells,1,drawCell) #draw all cells

#  # label cells by id number
# for (c in 1:nrow(cells)){
# 	xpt = cells[c,"x"]+cells[c,"s"]/2
# 	ypt = cells[c,"y"]+cells[c,"s"]/2
# 	show = paste(cells[c,"ID"])
# #	text(xpt,ypt,show)
# }

plot(premises[pToPlot,"x"], premises[pToPlot,"y"], xaxt="n", yaxt="n", xlab="", ylab="", 
	xlim=c(minx,maxx), ylim=c(miny,maxy), bty="n", pch="*", col="gray")
# draw cells on top
apply(ucells,1,drawCell) #draw all cells
