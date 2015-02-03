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
    N=4000; % number of farms
    Size=200;
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

x=Size*rand(N,1); y=Size*rand(N,1); 
%x=Size/2*rand(N,1); y=Size/2*rand(N,1); KT
Status=zeros(N,1); Status(1:Y0)=1;

% vectors of x and y coordinates are 4000 random numbers sized up to 20 - maintains a
% farm/grid cell ratio
R=rand(N,1); 
Cows=zeros(N,1); % vector of # of cows on each farm
Sheep=zeros(N,1); % vector of # of sheep on each farm
n=find(R<0.73); % 73% of farms have cows
m=find(R<0.46 | R>0.73); % 46% have cows and sheep, the remaining are sheep only?
while length(m)+length(n)>0
    Sheep(m)=350*exp(randn(size(m)));
    Cows(n)=70*exp(randn(size(n)));
    m=m(find(Sheep(m)<1 | Sheep(m)>30000));
    n=n(find(Cows(n)<1 | Cows(n)>5000));
end
Suscept=Sheep+10.5*Cows; % suscep for each farm based on cows & sheep
Transmiss=5.1e-7*Sheep + 7.7e-7*Cows; % trans for each farm based on cows & sheep

disp('Total animals: ')
disp(sum(Cows)+sum(Sheep))

gridsize = 20;
grid=WhichGrid(x,y,Size,Size,gridsize,gridsize); % find grid cell for this location
[tmp,i]=sort(grid);
x=x(i); y=y(i); Status=Status(i); grid=grid(i);
for i=1:max(grid) % for each grid cell
    Xgrid(i)=floor((i-1)/gridsize);
    Ygrid(i)=mod((i-1),gridsize);
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
            Dist2=(Size*max([0 (abs(Xgrid(i)-Xgrid(j))-1)])/gridsize).^2 + (Size*max([0 (abs(Ygrid(i)-Ygrid(j))-1)])/gridsize).^2;
            MaxRate(i,j)=Max_Sus_grid(j)*Kernel(Dist2);
        end
    end
end

for r=1:100

% Susceptible, Exposed, Infectious, Reported.
t=0; i=1; S=length(find(Status==0)); E=length(find(Status>0 & Status<=5)); 
I=length(find(Status>5 & Status<=9)); R=length(find(Status==10)); R2=length(find(Status>9)); CullSheep=0; CullCattle=0;
i=i+1;  IterateFlag=1;

disp('Gridding: ')
% The main iteration
while (t<MaxTime & IterateFlag)

    [Status]=Iterate(Status, x, y, Suscept, Transmiss, RingCull, grid, first_in_grid, last_in_grid, Num, MaxRate);

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
    
end
    disp('Sheep: ')
    disp(sum(CullSheep))
    disp('Cattle: ')
    disp(sum(CullCattle))
temp = [max(t) CullSheep(i-1) CullCattle(i-1)]; % create temp row with time, sheep/cattle culled
% write (append) allCulled to text file
fID = fopen('matlab_mkgrid.txt','a'); % open file for appending
fformat = '%d\t%f\t%f\n'; % format as fixed decimal, tab-separated, with line break
fprintf(fID,fformat,temp); % print to file
fclose(fID); % close file

disp('Pairwise: ')
Status=zeros(N,1); Status(1:Y0)=1;
% Susceptible, Exposed, Infectious, Reported.
t=0; i=1; S=length(find(Status==0)); E=length(find(Status>0 & Status<=5)); 
I=length(find(Status>5 & Status<=9)); R=length(find(Status==10)); R2=length(find(Status>9)); CullSheep=0; CullCattle=0;
i=i+1;  IterateFlag=1;

% The main iteration
while (t<MaxTime & IterateFlag)

    [Status]=IteratePW(Status, x, y, Suscept, Transmiss, RingCull);%, grid, first_in_grid, last_in_grid, Num, MaxRate);

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
    
end
    disp('Sheep: ')
    disp(sum(CullSheep))
    disp('Cattle: ')
    disp(sum(CullCattle))
temp = [max(t) CullSheep(i-1) CullCattle(i-1)]; % create temp row with time, sheep/cattle culled
% write (append) allCulled to text file
fID = fopen('matlab_ktpairwise.txt','a'); % open file for appending
fformat = '%d\t%f\t%f\n'; % format as fixed decimal, tab-separated, with line break
fprintf(fID,fformat,temp); % print to file
fclose(fID); % close file

end

% Iteration Space
function [Status]=Iterate(Status, x, y, Suscept, Transmiss, RingCull, grid, first_in_grid, last_in_grid, Num, MaxRate)

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
            Q=1-exp(-Transmiss(INFi)*Suscept(ind).*Kernel((x(INFi)-x(ind)).^2+(y(INFi)-y(ind)).^2));
            Event(ind(find(rand(size(Q))-Q<0 & Status(ind)==0)))=1;
        else
            for j=1:Num(M)
                ind=first_in_grid(M)+j-1;
                P=1-s*(1-PAB).^(Num(M)+1-j);
                R=rand(1,1);
                if R<PAB/P
                    s=0;
                    Q=1-exp(-Transmiss(INFi)*Suscept(ind).*Kernel((x(INFi)-x(ind)).^2+(y(INFi)-y(ind)).^2));
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

% Iteration Space - pairwise
function [Status]=IteratePW(Status, x, y, Suscept, Transmiss, RingCull);%, grid, first_in_grid, last_in_grid, Num, MaxRate)

Event=0*Status; % initialize with 0s

INF=find(Status>5 & Status<12);  NI=length(INF); % Note reported farms still infectious
%IGrids=grid(INF);
SUS=find(Status==0); NS=length(SUS); %kt

for i=1:NI
    INFi=INF(i); % for each infectious farm
%    MaxProb=1-exp(-Transmiss(INFi)*Num.*MaxRate(IGrids(i),:));
    % transmissibility of this farm.... max rate for this farm's grid
%    m=find(MaxProb-rand(1,max(grid))>0);  % these are grids that need further consideration
    % which grids' probabilities are successful
    for i2=1:NS
        SUSi=SUS(i);
    
        R=rand(1,1); %kt
        P=1-exp(-Transmiss(INFi)*Suscept(SUSi).*Kernel((x(INFi)-x(SUSi)).^2+(y(INFi)-y(SUSi)).^2)); %kt
        if R<P
            Event(SUSi)=1;
        end
    end

end
m=find(Status>0);
Status(m)=Status(m)+1; % increase progression for non-susceptibles
Status=Status+Event; % implement new infections

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
% XRange and YRange are both 20
% XNum and YNum are both 10

G=floor(x*XNum/XRange)*YNum+floor(y*YNum/YRange)+1;
% x values * number of farms/10 (grid size)



% Does a simple check on the value
function []=CheckGreaterOrEqual(Parameter, Value, str)
m=find(Parameter<Value);
if ~isempty(m)
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
