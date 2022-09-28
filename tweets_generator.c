#include "markov_chain.h"

#include <unistd.h>
#include <string.h>

/***************************/
/*   CONSTANT VARIABLES    */
/***************************/

#define ARG_NUM_DEFLT 4
#define ARG_NUM_OPTN 5
#define FILE_PATH_ARG 3
#define SEED_ARG 1
#define TWEETS_NUM_ARG 2
#define WORD_LIMIT_ARG 4
#define MAX_LINE_SIZE 1001
#define WHITESPACES " \r\n"
#define PERIOD '.'
#define MAX_TWEET_LEN 20
#define NO_LIMIT -1


/***************************/
/*     ERROR MESSAGES      */
/***************************/

#define ARG_NUM_ERR "Usage: input seed, # of tweets,"\
" txt file, and optionally max # words to read from file."
#define FILE_ERR "Error: file unreadable or does not exist"
#define ALLOCATION_ERR "Allocation failure: there has been a memory allocatio"\
"n failure."


/***************************/
/*       FUNCTIONS         */
/***************************/

static void print_func (void *data_ptr)
{
  if (((char *) data_ptr)[strlen ((char *) data_ptr) - 1] == PERIOD) {
    printf("%s", (char *) data_ptr);
    return;
  }
  printf ("%s ", (char *) data_ptr);
}

static int comp_func (void *data1, void *data2)
{
  char *str1 = (char *) data1;
  char *str2 = (char *) data2;
  int is_equal = strcmp (str1, str2);
  return is_equal;
}

static void free_data (void *data_ptr)
{
  if (data_ptr)
  {
    free ((char *) data_ptr);
    data_ptr = NULL;
  }
}

static void *copy_func (void *data_ptr)
{
  void *data_cpy = malloc (strlen ((char *) data_ptr) + 1);
  if (!data_cpy) {
    return NULL;
  }
  data_cpy = memcpy (data_cpy, data_ptr, strlen ((char *) data_ptr) + 1);
  return data_cpy;
}

static bool is_last (void *data_ptr)
{
  if (((char *) data_ptr)[strlen ((char *) data_ptr) - 1] == PERIOD)
  {
    return true;
  }
  return false;
}

/**
 * checks if file path is valid
 * @param argv file path
 * @return true if valid, else false
 */
static bool check_file_path (char argv[])
{
  if (access (argv, F_OK | R_OK) == -1)
  {
    return false;
  }
  return true;
}

/**
 * checks that number of args is valid
 * @param argc num of args
 * @return true if valid, else false
 */
static bool check_num_args (int argc)
{
  if (argc != ARG_NUM_DEFLT && argc != ARG_NUM_OPTN)
  {
    return false;
  }
  return true;
}

/**
 * checks if args are valid, if not prints error message
 * @param argc num of args
 * @param arguments ptr to array of args
 * @return true if valid, else false
 */
static bool check_args (int argc, char **arguments)
{
  if (!check_num_args (argc))
  {
    printf (ARG_NUM_ERR);
    return false;
  }
  if (!check_file_path (arguments[FILE_PATH_ARG]))
  {
    printf (FILE_ERR);
    return false;
  }
  return true;
}

/**
 * sets seed by input
 * @param seed_arg
 */
static void set_seed (char *seed_arg)
{
  int seed;
  sscanf (seed_arg, "%i", &seed);
  srand (seed);
}

/**
 * fills markov chain from text
 * @param fp text file
 * @param words_to_read limit on how many words to read
 * @param markov_chain
 * @return 0 if failed to allocate memory, else 1
 */
static int
fill_database (FILE *fp, int words_to_read, MarkovChain *markov_chain)
{
  char line[MAX_LINE_SIZE];
  char delimit[] = WHITESPACES; //where to cut up sentence
  int words_read = 0;
  while ((words_read != words_to_read) && fgets (line, sizeof (line), fp))
  {
    char *token = strtok (line, delimit); //first word
    char *prev_token = NULL;
    while (token && words_read != words_to_read)
    {
      if (words_to_read >= 0) //if user entered word limit
      {
        words_read++;
      }
      Node *node = add_to_database (markov_chain, token);
      if (!node) {
        free_markov_chain (&markov_chain);
        return 0;
      }
      MarkovNode *m_node = node->data;
      if (prev_token)
      {
        Node *prev_m_node = get_node_from_database (markov_chain,
                                                    prev_token);
        if (!add_node_to_counter_list (prev_m_node->data, m_node,
                                       markov_chain))
        {
          free_markov_chain (&markov_chain); //free allocated memory
          return 0;
        }
      }
      prev_token = token;
      token = strtok (NULL, WHITESPACES); //get next word
    }
  }
  return 1;
}

int main (int argc, char *argv[])
{
  if (!check_args (argc, argv))
  {
    return EXIT_FAILURE; //if args invalid
  }
  set_seed (argv[SEED_ARG]); //set seed
  int tweets_num;
  sscanf (argv[TWEETS_NUM_ARG], "%d", &tweets_num); //number of tweets to gen
  FILE *input = fopen (argv[FILE_PATH_ARG], "r");
  int word_limit = NO_LIMIT; //default to no limit on words to read
  if (argc == ARG_NUM_OPTN) //if 4th arg exists
  {
    sscanf (argv[WORD_LIMIT_ARG], "%d", &word_limit); //update word limit
  }
  LinkedList linked_list = {NULL, NULL, 0}; //init new linked list
  MarkovChain markov_chain = {&linked_list, &print_func, &comp_func,
                              &free_data, &copy_func,
                              &is_last}; //init new markov chain
  if (!fill_database (input, word_limit, &markov_chain)) //fill linked list
  {
    printf (ALLOCATION_ERR); //failed to allocate memory at any point
    return EXIT_FAILURE; //memory freed in fill_database function
  }
  fclose (input); //close text file
  for (int i = 0; i < tweets_num; i++) //print desired num of tweets
  {
    printf ("Tweet %d: ", i + 1);
    generate_random_sequence (&markov_chain, NULL, MAX_TWEET_LEN - 1);
    printf ("\n");
  }
  MarkovChain *chain_ptr = &markov_chain;
  free_markov_chain (&chain_ptr); //free allocated memory
  return EXIT_SUCCESS;
}
