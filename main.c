#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <memory.h>

// data structure for the board
struct board {
  int width;
  int height;
  char *cells;
};

struct board *board_create(int width, int height) {
  struct board *ret = (struct board*)malloc(sizeof(struct board));
  ret->width=width;
  ret->height=height;
  ret->cells=(char*)malloc(width*height);
  memset(ret->cells,0,width*height);
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

int validsplit(int min, int max, int split, int height) {
  int area = (split-min)*height;
  int area2= (max-split)*height;
  return area>2 && area2>2;
}

int randcoord(int from, int to) {
  return rand() % (to-from) + from;
}

void board_generate(struct board *board, int l, int b, int r, int t) {
  int xsplit=l, ysplit=r;
  /* find split...*/
  xsplit = randcoord(l,r);
  ysplit = randcoord(b,t);
  /* check if a split occurre */
  if (validsplit(l,r,xsplit,t-b)) {
    board_generate(board,l,b,xsplit,t);
    board_generate(board,xsplit,b,r,t);
  } else if (validsplit(b,t,ysplit,r-l)) {
    board_generate(board,l,b,r,ysplit);
    board_generate(board,l,ysplit,r,t);
  } else {
    /* pick a random square and record the area there.. */
    int xcoord = randcoord(l,r);
    int ycoord = randcoord(b,t);
    board_set(board,xcoord,ycoord,(t-b)*(r-l));
  }
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
  float rightedge=8;
  float topedge=11;
  float xscale=rightedge*72.f/(board->width+1);
  float yscale=topedge*72.f/(board->height+1);
  float scale=xscale < yscale ? xscale : yscale;
  float fontsize = 0.5f; /* in cell units */
  printf("%%!PS-Adobe-3.0\n");
  printf("<< /PageSize [%f %f] >> setpagedevice\n",rightedge*72.f,topedge*72.f);
  printf("%f %f scale\n",scale,scale);
  printf("0.5 0.5 translate\n");
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
  printf("showpage\n");
}

int main() {
  struct board *board = board_create(10,10);
  board_generate(board,0,0,board->width,board->height);
  /*board_print(board);*/
  board_ps(board);
  return 0;
}
