setwd("/Users/kimtsao/Desktop/PremModel/grid-test")

premises = read.delim("testprems.txt",header=F)
	colnames(premises) = c("ID","x","y","pop")
	
cells = read.delim("cellList.txt",header=F)
	colnames(cells) = c("ID","x","y","s","farms")
	
drawCell <- function(cell) { # cell is row from cells
	xvec = c(cell["x"], cell["x"]+cell["s"], cell["x"]+cell["s"], cell["x"])
	yvec = c(cell["y"], cell["y"], cell["y"]+cell["s"], cell["y"]+cell["s"])
	polygon(xvec,yvec,border="red")
}
	
# pick a random subset of premises to plot
#pToPlot = sample(1:nrow(premises), 1000, replace=F)

# plot every 100th prem
pToPlot = seq(1,nrow(premises),by=100)
	
# plot premises
plot(premises[pToPlot,"x"], premises[pToPlot,"y"], bty="n", pch="*", col="gray")
# draw cells on top
apply(cells,1,drawCell)
# label cells by id number
for (c in 1:nrow(cells)){
	xpt = cells[c,"x"]+cells[c,"s"]/2
	ypt = cells[c,"y"]+cells[c,"s"]/2
	show = paste(cells[c,"ID"])
	text(xpt,ypt,show)
}