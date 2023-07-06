#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <limits.h>


void read_rawimage(fname, length, width, image)
    char *fname; unsigned long length; unsigned long width; unsigned char image[length][width];
{
        short i;
        FILE *file;

        file=fopen(fname,"r");
        for (i=0; i<length; i++)
                        fread(image[i], 1, width, file);
        fclose(file);
}

void write_rawimage(fname, length, width, image)
    char *fname; unsigned long length; unsigned long width; unsigned char image[length][width];
{
        short i;
        FILE *file;

        file=fopen(fname,"w");
        for (i=0; i<length; i++)
                       fwrite(image[i], 1, width, file);


        fclose (file);
}
//filter
void copy_in_2_out_img (length, width, inimg, outimg)
    unsigned long length, width;
    unsigned char inimg[length][width], outimg[length][width];
{
	int total=0;
	int top=256;
	int sumB=0;
	int wB=0;
	int maximum = INT_MIN;
	int sum1=0;

	int hist[256];
	int i,j,temp;
	double start,end;
	start = omp_get_wtime();

	for (i=0;i<=255;i++)
		hist[i] = 0;
		
	for(i=0;i<length;i++)
    {
    for(j=0;j<width;j++)
    {
      temp = inimg[i][j];
      hist[temp] += 1;
    }
    }


for(i=0;i<top;i++)
{
	sum1=sum1+i*hist[i];
	total=total+hist[i];
}


int wF,mF;
int level,val;
for(i=1;i<=top;i++)
{
    wB +=hist[i];
    sumB += (i-1)*hist[i];
    wF=total-wB;
	if(wB>0 && wF>0)
{
    	mF=(sum1-sumB)/wF;
	val=wB*wF*((sumB/wB)-mF)*((sumB/wB)-mF);
	if(val>=maximum)
{			
    level=i;
	maximum=val;
}
}
}



	for(i=0;i<length;i++)
  {
    for(j=0;j<width;j++)
    {
    	if(inimg[i][j]<level)
		{
    	outimg[i][j]=0;
		}
    	else
		{
    		outimg[i][j]=255;
		}
    }
}
end = omp_get_wtime();
printf("Serial time: %f s\n",end - start);

int P;
for(P = 2; P < 65;P *= 2)
{
	double start2,end2;
	int total=0;
	int top=256;
	int maximum = INT_MIN;

	int hist[256];
	int i,j,temp;
	start2 = omp_get_wtime();

	for (i=0;i<=255;i++)
		hist[i] = 0;

	#pragma omp parallel for num_threads(P) private(i,j,temp) collapse(2) \
	reduction(+:hist) schedule(static,length*width/P)
	for(i=0;i<length;i++)
    for(j=0;j<width;j++)
    {
        temp = inimg[i][j];
	hist[temp] += 1;
    }

#pragma omp parallel num_threads(P) private(i,hist) reduction(+:wB,sumB,sum1,total)
{
    #pragma omp for schedule(static,top/P)
    for (int i = 0; i < top; i++) {
        sum1 += i * hist[i];
        total += hist[i];
    }

    #pragma omp for schedule(static,top/P)
    for (int i = 1; i <= top; i++) {
        wB += hist[i];
        sumB += (i-1)*hist[i];
        int wF = total - wB;
        if (wB > 0 && wF > 0) {
    	int mF=(sum1-sumB)/wF;
	int val=wB*wF*((sumB/wB)-mF)*((sumB/wB)-mF);
            if (val > maximum) {
                maximum = val;
                level = i;
            }
        }
    }
       #pragma omp for collapse(2) schedule(static,length*width/P)
	for(i=0;i<length;i++)
    	for(j=0;j<width;j++)
    	{
    	if(inimg[i][j]<level)
		{
    	outimg[i][j]=0;
		}
    	else
		{
    		outimg[i][j]=255;
		}
    	}
        }
	end2= omp_get_wtime();
	printf("Parallel time for %d threads: %.6f s\n",P,end2 - start2);
	printf("Speedup: %.4f\n",(end-start)/(end2-start2));
	printf("Efficiency: %.4f\n",(end - start)/(P*(end2 - start2)));
	printf("\n");
}
}
// τελος φιλτρου 
void main(int argc, char *argv[]) {
    char infname[50], outfname[50];
    unsigned char **inimg, **outimg;
    unsigned long height, width, i, j;

    if (argc<4) {printf("usage is: %s inimg height width [outimg]\n", argv[0]);exit(-1);}
    strcpy(infname,argv[1]);
    height=(unsigned long)atoi(argv[2]);
    width=(unsigned long)atoi(argv[3]);
    strcpy(outfname,argv[4]);
    
    inimg = (unsigned char **) malloc (height*width);
    read_rawimage(infname, height, width, inimg);



    outimg = (unsigned char **) malloc (height*width);
      
  

  

    copy_in_2_out_img (height, width, inimg, outimg);



  
    write_rawimage(outfname, height, width, outimg);
    free(outimg);
		
    free(inimg);
}


