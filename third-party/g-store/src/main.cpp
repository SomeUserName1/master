//#include <stdio.h>
//#include <string.h>
#include <Windows.h>

#include "defs.h"
#include "structs.h"
#include "parameters.h"
#include "cmd_input.h"
#include "entry_points.h"

//#include <g-store.h>

//char MY_NAME[];

int main(int argc, char *argv[])
{
  if (argc == 1)
    printf("*** G-Store - A Storage Manager for Graph Data (version 0.12)\n\n");

  if (argc == 2 && strcmp(argv[1], "q") == 0)
    Sleep(200);

  load_parameters();
  load_db_globals();
 
  if (argc == 1)
  {
    if (*GLOBALS::graph_name != '\0')
      printf("...Loaded graph %s\n\n", GLOBALS::graph_name);
    
    printf("Type HELP; to get help\n\n\n");
  }

  if (argc == 1 || (argc == 2 && strcmp(argv[1], "q") == 0))
  {
    sprintf(MY_NAME, "%s q", argv[0]);  //used for restarting the program in case of error
    for(;;)
      read_menu();
  }
  else
  {
    *MY_NAME = '\0';
    read_argument(argv[1]);
  }
    
  close_down();
  
  assert(false);
  return 0;
}

