#ifndef _GLOBAL_DEF
#define _GLOBAL_DEF
/* The image header data structure      */
class header {
public:
	int nr, nc;             /* Rows and columns in the image */
	int oi, oj;             /* Origin */
};

/*      The IMAGE data structure        */
class image {
public:
		header *info;            /* Pointer to header */
		unsigned char **data;    /* Pixel values */
		image(){data=NULL;info=NULL;}
		~image(){
			if(data!=NULL){
				for(int i=0;i<info->nr;i++)
					delete [] data[i];
				delete [] data;
			}
			if(info!=NULL){
				delete info;
				info=NULL;
			}
		}
};
typedef image* IMAGE;
#endif