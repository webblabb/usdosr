setwd("/Users/kimtsao/Desktop/PremModel/grid-test")

premises = read.delim("heterotest.txt",header=F)
	colnames(premises) = c("ID","x","y","pop") # x-y order reversed in FLAPS?

#premises = read.delim("USprems.txt",header=F)
#	colnames(premises) = c("ID","y","x","pop") # x-y order reversed in FLAPS?
	
cells = read.delim("cellList_7772cells.txt",header=F)
	colnames(cells) = c("ID","x","y","s","farms")
	
drawCell <- function(cell, color="red") { # cell is row from cells
	xvec = c(cell["x"], cell["x"]+cell["s"], cell["x"]+cell["s"], cell["x"])
	yvec = c(cell["y"], cell["y"], cell["y"]+cell["s"], cell["y"]+cell["s"])
	polygon(xvec,yvec,border=color)
}
	
# pick a random subset of premises to plot
#pToPlot = sample(1:nrow(premises), 1000, replace=F)

# plot 1000 prems
pToPlot = seq(1,nrow(premises),length.out=1000) # equal intervals
	
# plot premises
plot(premises[pToPlot,"x"], premises[pToPlot,"y"], bty="n", pch="*", col="gray")
# draw cells on top
#apply(cells,1,drawCell) #draw all cells

#neighbors
drawCell(cells[6168+1,],"green")
drawCell(cells[6169+1,],"green")
drawCell(cells[6170+1,],"green")
drawCell(cells[6173+1,],"green")
drawCell(cells[6135+1,],"green")
drawCell(cells[6134+1,],"green")
drawCell(cells[5996+1,],"green")
drawCell(cells[5990+1,],"green")
drawCell(cells[5988+1,],"green")
drawCell(cells[5966+1,],"green")

#focal
drawCell(cells[6167+1,],"blue")

# label cells by id number
for (c in 1:nrow(cells)){
	xpt = cells[c,"x"]+cells[c,"s"]/2
	ypt = cells[c,"y"]+cells[c,"s"]/2
	show = paste(cells[c,"ID"])
#	text(xpt,ypt,show)
}
