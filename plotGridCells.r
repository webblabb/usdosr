setwd("/Users/kimtsao/Desktop/PremModel/grid-test")

#premises = read.delim("heterotest.txt",header=F)
#	colnames(premises) = c("ID","x","y","pop") # x-y order reversed in FLAPS?

premises = read.delim("USprems.txt",header=F)
	colnames(premises) = c("ID","y","x","pop") # x-y order reversed in FLAPS?
	
cells = read.delim("max250f_7328c_USprems.txt",header=F)
	colnames(cells) = c("ID","x","y","s","farms")
	
drawCell <- function(cell, color="red") { # cell is row from cells
	xvec = c(cell["x"], cell["x"]+cell["s"], cell["x"]+cell["s"], cell["x"])
	yvec = c(cell["y"], cell["y"], cell["y"]+cell["s"], cell["y"]+cell["s"])
	polygon(xvec,yvec,border=color)
}

# plot 1000 prems picked at intervals
pToPlot = seq(1,nrow(premises),length.out=1000) # equal intervals
	
# plot premises
plot(premises[pToPlot,"x"], premises[pToPlot,"y"], bty="n", pch="*", col="gray")
# draw cells on top
apply(cells,1,drawCell) #draw all cells

# label cells by id number
for (c in 1:nrow(cells)){
	xpt = cells[c,"x"]+cells[c,"s"]/2
	ypt = cells[c,"y"]+cells[c,"s"]/2
	show = paste(cells[c,"ID"])
#	text(xpt,ypt,show)
}
