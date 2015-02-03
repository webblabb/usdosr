function [t,S,I] = Program_7_6(N, Size, Y0, RingCull, MaxTime)
%
%
%
% Program_7_6( N, size, Y0, RingSize, MaxTime)
%      This is the MATLAB version of program 7.6 from page 274 of
% "Modeling Infectious Disease in humans and animals"
% by Keeling & Rohani.
%
% It is an individual-based epidemic model, which corresponds closely to
% the foot-and-mouth epidemic in the UK. N farms are randomly distributed
% in a size x size space (in km^2) and Y0 are assumed to be initially infectious.
% The farm-size, susceptiblity and transmissibility parameters mimic the
% known UK situation.
% A ring-cull policy around identified herds can be utilised, RingCull 
% sets the radius of the ring. As an example you may wish to try:
% Program_7_6(4000,20,5,1,1000);
%
% A one-day time step is used together with the short-cuts mentioned in 
% Box 7.3

% Sets up default parameters if necessary.
if nargin == 0
    N=4000;
    Size=20;
    Y0=1;
    RingCull=0;
    MaxTime=1000;
end

% Checks all the parameters are valid
CheckGreater(N,0,'Individuals N ');
CheckGreater(Size,0,'Size of the space');
CheckGreater(Y0,0,'number of infections Y0');
CheckGreaterOrEqual(RingCull,0,'Radius of ring cull');

% Set Up a 10x10 grid and position individuals
%
% Status=0     => Susceptible
% Status=1-5   => Exposed
% Status=6-11  => Infectious
% Status=10-11 => Reported but still Infectious
% Status=12-   => Culled but waiting local ring culling
% Status=-1    => Culled

x=Size*rand(N,1); y=Size*rand(N,1); Status=zeros(N,1); Status(1:Y0)=1;
R=rand(N,1); Cows=zeros(N,1); Sheep=zeros(N,1);
n=find(R<0.73); m=find(R<0.46 | R>0.73);
while length(m)+length(n)>0
    Sheep(m)=350*exp(randn(size(m)));
    Cows(n)=70*exp(randn(size(n)));
    m=m(find(Sheep(m)<1 | Sheep(m)>30000));
    n=n(find(Cows(n)<1 | Cows(n)>5000));
end
Suscept=Sheep+10.5*Cows; Transmiss=5.1e-7*Sheep + 7.7e-7*Cows;

grid=WhichGrid(x,y,Size,Size,10,10);
[tmp,i]=sort(grid);
x=x(i); y=y(i); Status=Status(i); grid=grid(i);
for i=1:max(grid)
    Xgrid(i)=floor((i-1)/10);
    Ygrid(i)=mod((i-1),10);
    m=find(grid==i);
    Num(i)=length(m);
    if Num(i)>0
        first_in_grid(i)=min(m);
        last_in_grid(i)=max(m);
        Max_Sus_grid(i)=max(Suscept(m));
    else
        first_in_grid(i)=0;
        last_in_grid(i)=-1;
        Max_Sus_grid(i)=0;
    end
end

%Work out grid to maximum grid transmission probabilities
for i=1:max(grid)
    for j=1:max(grid)
        if i==j | Num(i)==0 | Num(j)==0
            MaxRate(i,j)=inf;
        else
            Dist2=(Size*max([0 (abs(Xgrid(i)-Xgrid(j))-1)])/10).^2 + (Size*max([0 (abs(Ygrid(i)-Ygrid(j))-1)])/10).^2;
            MaxRate(i,j)=Max_Sus_grid(j)*Kernel(Dist2);
        end
    end
end

% Susceptible, Exposed, Infectious, Reported.
t=0; i=1; S=length(find(Status==0)); E=length(find(Status>0 & Status<=5)); 
I=length(find(Status>5 & Status<=9)); R=length(find(Status==10)); R2=length(find(Status>9)); CullSheep=0; CullCattle=0;
i=i+1;  IterateFlag=1;
% The main iteration
all_dists = [];
all_Q = [];
while (t<MaxTime & IterateFlag)

    [Status, all_dists, all_Q]=Iterate(Status, x, y, Suscept, Transmiss, RingCull, grid, first_in_grid, last_in_grid, Num, MaxRate, all_dists, all_Q);

    Sus=find(Status==0); Exp=find(Status>0 & Status<=5); Inf=find(Status>5 & Status<=9); 
    Rep=find(Status>9); Culled=find(Status<0);
    S(i)=length(Sus); E(i)=length(Exp); I(i)=length(Inf); R(i)=length(find(Status==10)); R2(i)=length(Rep);
    CullSheep(i)=sum(Sheep(Culled)); CullCattle(i)=sum(Cows(Culled));
    t(i)=t(i-1)+1; i=i+1;

    if t(end)>5
        if (E(end-3)+I(end-3)+R2(end-3))==0
            IterateFlag=0;
        end
    end
    
%     % plots the data
%     subplot(2,1,1);
%     % Set-up points just for legend.
%     h(5)=plot(-Size,-Size,'.k','MarkerSize',10);
%     hold on;
%     h(1)=plot(-Size,-Size,'ok','MarkerFaceColor','g','MarkerSize',6);
%     h(2)=plot(-Size,-Size,'ok','MarkerFaceColor',[0.9 0.9 0],'MarkerSize',8);
%     h(3)=plot(-Size,-Size,'ok','MarkerFaceColor',[1 0.5 0],'MarkerSize',10);
%     h(4)=plot(-Size,-Size,'sk','MarkerFaceColor','r','MarkerSize',10);
%    
%     plot(x(Culled),y(Culled),'.k','MarkerSize',10);
%     plot(x(Sus),y(Sus),'ok','MarkerFaceColor','g','MarkerSize',6);
%     plot(x(Exp),y(Exp),'ok','MarkerFaceColor',[0.9 0.9 0],'MarkerSize',8);
%     plot(x(Inf),y(Inf),'ok','MarkerFaceColor',[1 0.5 0],'MarkerSize',10);
%     plot(x(Rep),y(Rep),'sk','MarkerFaceColor','r','MarkerSize',10);
%     hold off
%     axis([0 Size 0 Size]);
%     legend(h,'Susceptible','Exposed','Infectious','Reported','Culled',-1);
% 
%     subplot(4,1,3);
%     h=plot(t,E,'-y',t,I,'-r',t,R,'-r');
%     set(h(1),'Color',[0.9 0.9 0]);
%     set(h(2),'Color',[1 0.5 0]);
%     set(h(3),'LineWidth',2);
%     legend(h,'Exposed','Infectious','Reports',2);
%     ylabel 'Total Infected'
% 
%     subplot(4,1,4);
%     h=plot(t,CullSheep,'-b',t,CullCattle,'-g');
%     legend(h,'Sheep','Cattle',2);
%     ylabel 'Total Animals Culled'
%     xlabel 'Time (days)'
%     drawnow
    
end

scatter(all_dists, all_Q);

% Iteration Space
function [Status, all_dists, all_Q]=Iterate(Status, x, y, Suscept, Transmiss, RingCull, grid, first_in_grid, last_in_grid, Num, MaxRate, all_dists, all_Q)

Event=0*Status;

INF=find(Status>5 & Status<12);  NI=length(INF); % Note reported farms still infectious
IGrids=grid(INF);

for i=1:NI
    INFi=INF(i);
    MaxProb=1-exp(-Transmiss(INFi)*Num.*MaxRate(IGrids(i),:));
    m=find(MaxProb-rand(1,max(grid))>0);  % these are grids that need further consideration

    for n=1:length(m)
        s=1;
        M=m(n);
        PAB=1-exp(-Transmiss(INFi)*MaxRate(IGrids(i),M));
        if PAB==1
            ind=first_in_grid(M):last_in_grid(M);
            d_sq = (x(INFi)-x(ind)).^2+(y(INFi)-y(ind)).^2;
            d = sqrt(d_sq);
            all_dists = [all_dists, d'];
            Q=1-exp(-Transmiss(INFi)*Suscept(ind).*Kernel(d_sq));
            all_Q = [all_Q, Q'];
            Event(ind(find(rand(size(Q))-Q<0 & Status(ind)==0)))=1;
        else
            for j=1:Num(M)
                ind=first_in_grid(M)+j-1;
                P=1-s*(1-PAB).^(Num(M)+1-j);
                R=rand(1,1);
                if R<PAB/P
                    s=0;
                    d_sq = (x(INFi)-x(ind)).^2+(y(INFi)-y(ind)).^2;
                    d = sqrt(d_sq);
                    all_dists = [all_dists, d'];
                    Q=1-exp(-Transmiss(INFi)*Suscept(ind).*Kernel(d_sq));
                    all_Q = [all_Q, Q'];
                    if R<Q/P & Status(ind)==0
                        Event(ind)=1;
                    end
                end
            end
        end
    end
end
m=find(Status>0);
Status(m)=Status(m)+1;
Status=Status+Event;

m=find(Status==13); % Initiate Ring Culling Around Reported Farm
for i=1:length(m)
    Status(m(i))=-1;
    D=(x(m(i))-x(:)).^2+(y(m(i))-y(:)).^2;
    n=find(D<RingCull.^2);
    Status(n)=-1;
end


% Calculates the transmission kernel
function K=Kernel(dist_squared)

P=[-9.2123e-5  9.5628e-4  3.3966e-3 -3.3687e-2 -1.30519e-1 -0.609262 -3.231772];

K=exp(polyval(P,dist_squared));

K(find(dist_squared<0.0138))=0.3093;
K(find(dist_squared>60*60))=0;



%Calculates which grid a particular location is in
function G=WhichGrid(x,y,XRange,YRange,XNum,YNum)

G=floor(x*XNum/XRange)*YNum+floor(y*YNum/YRange)+1;


% Does a simple check on the value
function []=CheckGreaterOrEqual(Parameter, Value, str)
m=find(Parameter<Value);
if length(m)>0
    error('Parameter %s(%g) (=%g) is less than %g',str,m(1),Parameter(m(1)),Value);
end

function []=CheckGreater(Parameter, Value, str)
m=find(Parameter<=Value);
if length(m)>0
    error('Parameter %s(%g) (=%g) is less than or equal to %g',str,m(1),Parameter(m(1)),Value);
end

function []=CheckLess(Parameter, Value, str)
m=find(Parameter>=Value);
if length(m)>0
    error('Parameter %s(%g) (=%g) is greater than or equal to %g',str,m(1),Parameter(m(1)),Value);
end
