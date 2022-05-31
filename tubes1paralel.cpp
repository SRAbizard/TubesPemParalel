#include<stdio.h>
#include<stdlib.h>
#include <mpi.h>
#define DEBUG
#define ROOT 0
#define ISPOWER2(x) (!((x)&((x)-1)))
/* Merge 2 arrays with same size */
int* merge(int array1[], int array2[], int size){
	int* result = (int*)malloc(2*size*sizeof(int));
	int i=0, j=0,k=0;
	while((i<size)&&(j<size))
	result[k++]=(array1[i]<=array2[j])? array1[i++] : array2[j++];
	
	while (i<size)
	result[k++]=array1[i++];
	
	while(j<size)
	result[k++]=array2[j++];
	
	return result;
}
/*validate sorted data */

int sorted(int array[], int size){
	int i;
	for (i=1;i<size;i++)
	if(array[i-1]>array[i])
	return 0;
	return 1;
}
/* Needed by qsort()*/
int compare(const void* p1, const void* p2){
	return *(int*)p1 - *(int*)p2;
}
int main(int argc, char** argv){
	int i, b=1,npes,myrank;
	long datasize;
	int localsize, *localdata, *otherdata, *data = NULL;
	int active=1;
	MPI_Status status;
	double start, finish, p, s;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &npes);
	/*Read datasize argument */
	datasize = strtol(argv[1], argv, 10);
	/*check argument*/
	if(!ISPOWER2(npes)){
		if(myrank==ROOT) printf("Nomer prosesor harus berkekuatan 2.\n");
		return MPI_Finalize();
	}
	if (datasize%npes !=0){
		if(myrank==ROOT)printf("Ukuran data harus dapat dibagi dengan nomor prosesor.\n");
		return MPI_Finalize();
	}
	/*Generate data*/
	if (myrank==ROOT){
		data=(int*)malloc(datasize*sizeof(int));
		for(i=0;i<datasize;i++)
		data[i]=rand()%99+1;
	}
	/*Start point of paralale processing*/
	start=MPI_Wtime();
	/*Scatter data*/
	localsize = datasize/npes;
	localdata = (int*)malloc(localsize*sizeof(int));
	MPI_Scatter(data, localsize, MPI_INT,localdata,localsize,MPI_INT,ROOT,MPI_COMM_WORLD);
	/*Sort localdata*/
	qsort(localdata, localsize, sizeof(int),compare);
	/*Merge sorted data*/
	while (b<npes){
		if(active){
			if((myrank/b)%2==1){
				MPI_Send(localdata, b*localsize,MPI_INT, myrank-b,1,MPI_COMM_WORLD);
				free(localdata);
				active=0;
			}
			else{
				otherdata=(int*)malloc(b*localsize*sizeof(int));
				MPI_Recv(otherdata,b*localsize,MPI_INT, myrank+b,1,MPI_COMM_WORLD, &status);
				localdata=merge(localdata, otherdata, b*localsize);
				free(otherdata);
			}
		}
		b<<=1;
	}
	/*End point of parallel processing*/
	finish=MPI_Wtime();
	/*Runtime and speed-up analysis*/
	if(myrank==ROOT){
		#ifdef DEBUG
		if(sorted(localdata,npes*localsize)){
			printf("\nParalellel sorting succeed.\n\n");
		}
		else{
			printf("\nParallel sorting failed. \n\n");
		}
		#endif
		free(localdata);
		p=finish-start;
		printf("Parallel : %.8f\n",p);
		/*Sequential sort*/
	start = MPI_Wtime();
	qsort(data, datasize, sizeof(int),compare);
	finish = MPI_Wtime();
	free(data);
	s=finish-start;
	printf("Squential:%.8f\n",s);
	printf("Speed-up:%.8f\n",s/p);
	}
return MPI_Finalize();

}
