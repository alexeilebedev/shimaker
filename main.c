#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <memory.h>

/* shikaku board generator 
  algo 0: subdivide on x first 
  algo 1: subdivide larger direction first
  chunk: minimum acceptable area for child
*/

// data structure for the board
struct board {
  int width;
  int height;
  int algo;
  int chunk;
  int seed;
  int *cells;
};

int imin(int x, int y) {
  return x<y ? x:y;
}

struct board *board_create(int width, int height, int algo) {
  struct board *ret = (struct board*)malloc(sizeof(struct board));
  ret->width=width;
  ret->height=height;
  ret->cells=(int*)calloc(width*height,sizeof(int));
  ret->algo=algo;
  ret->chunk =algo==0 ? 2 : imin(ret->width,ret->height)/4;
  ret->seed=0;
  return ret;
}

void board_delete(struct board *board) {
  free(board->cells);
  free(board);
}

int board_get(struct board *board, int x, int y) {
  assert((unsigned)(x)<(unsigned)(board->width));
  assert((unsigned)(y)<(unsigned)(board->height));
  return board->cells[y * board->width + x];
}

void board_set(struct board *board, int x, int y, int val) {
  board->cells[y * board->width + x] = val;
}

int validsplit(struct board *board, int min, int max, int split, int height) {
  int area = (split-min)*height;
  int area2= (max-split)*height;
  return area>board->chunk && area2>board->chunk;
}

int randcoord(int from, int to) {
  return rand() % (to-from) + from;
}

void board_generate(struct board *board, int l, int b, int r, int t) {
  int xsplit=l, ysplit=b;
  /* find split...*/
  if (board->algo==0) {
    xsplit = randcoord(l,r);
    ysplit = randcoord(b,t);
  } else if (board->algo==1) {
    /* algo1: split bigger coordinate */
    if (r-l > t-b) {
      xsplit = randcoord(l+1,r);
    } else {
      ysplit = randcoord(b+1,t);
    }
  }
  /* check if a split occurre */
  if (validsplit(board,l,r,xsplit,t-b)) {
    board_generate(board,l,b,xsplit,t);
    board_generate(board,xsplit,b,r,t);
  } else if (validsplit(board,b,t,ysplit,r-l)) {
    board_generate(board,l,b,r,ysplit);
    board_generate(board,l,ysplit,r,t);
  } else {
    /* pick a random square and record the area there.. */
    int xcoord = randcoord(l,r);
    int ycoord = randcoord(b,t);
    board_set(board,xcoord,ycoord,(t-b)*(r-l));
  }
}

void board_generate_full(struct board *board) {
  srand(board->seed);
  board_generate(board,0,0,board->width,board->height);
}

void board_print(struct board *board) {
  printf("width:%d  height:%d\n", board->width, board->height);
  for (int y=board->height-1; y>=0; y--) {
    for (int x=0; x<board->width; x++) {
      int value = board_get(board,x,y);
      if (value) {
	printf("%2d",value);
      } else {
	printf(". ");
      }
    }
    printf("|\n");
  }
}

void ps_lineto(float xfrom, float yfrom, float xto, float yto) {
  printf("newpath %f %f moveto %f %f lineto closepath stroke\n"
	 ,xfrom,yfrom,xto,yto);
}

void board_ps(struct board *board) {
  float rightedge=595;/*points*/
  float topedge=842;
  float margin=25;
  float xscale=(rightedge-margin*2)/(board->width+1.f);
  float yscale=(topedge-margin*4)/(board->height+2.f);// why 4??
  float scale=xscale < yscale ? xscale : yscale;
  float fontsize = 0.5f; /* in cell units */
  printf("%%!PS-Adobe-3.0\n");
  /*printf("%%%% /PageBoundingBox 0 0 %f %f\n",rightedge,topedge);*/
  printf("<< /PageSize [%f %f] >> setpagedevice\n",rightedge,topedge);
  printf("%f %f translate\n",margin,margin*3); // why 3??
  printf("%f %f scale\n",scale,scale);
  printf("0 0 setlinewidth\n");
  printf("/Times-Roman findfont %f scalefont setfont\n",fontsize);
  /* print the board, so that each cell is 1 unit across */
  /* print the grid */
  for (int x=0; x<=board->width; x++) {
    ps_lineto(x,0, x,board->height);
  }
  for (int y=0; y<=board->height; y++) {
    ps_lineto(0,y, board->width, y);
  }
  
  for (int y=0; y<board->height; y++) {
    for (int x=0; x<board->width; x++) {
      int val=board_get(board,x,y);
      if (val) {
	printf("%f %f moveto (%d) dup stringwidth pop 2 div neg 0 rmoveto show\n", x+0.5f, y+0.3f, val);
      }
    }
  }
  // show definition in 12 point font
  printf("/Times-Roman findfont %f scalefont setfont\n",12.f/scale);
  printf("0 %f moveto (shimaker  width:%d  height:%d  algo:%d  seed:%d  chunk:%d) show\n"
	 ,board->height+0.3f, board->width, board->height, board->algo, board->seed, board->chunk);
  printf("showpage\n");
}

int main(int argc, char **argv) {
  int width=argc >1 ? atoi(argv[1]) : 10;
  int height=argc >2 ? atoi(argv[2]) : 10;
  int algo=argc > 3 ? atoi(argv[3]) : 0;
  int seed=argc > 4 ? atoi(argv[4]) : 0;
  struct board *board = board_create(width,height,algo);
  board->seed=seed;
  board_generate_full(board);
  /*board_print(board);*/
  board_ps(board);
  return 0;
}
