#include "markov_chain.h"
#include <stdlib.h>
#include <string.h>

/**
* Get random number between 0 and max_number [0, max_number).
* @param max_number maximal number to return (not including)
* @return Random number
*/
int get_random_number (int max_number)
{
  return rand () % max_number;
}

// full documentation in header file
MarkovNode *get_first_random_node (MarkovChain *markov_chain)
{
  int rand_num;
  Node *current_node;
  MarkovNode *rand_markov_node = NULL;
  while (!rand_markov_node || !rand_markov_node->counter_list)
  {
    current_node = (markov_chain->database->first);
    rand_num = get_random_number (markov_chain->database->size);
    for (int i = 0; i < rand_num; i++)
    {
      current_node = current_node->next;
    }
    rand_markov_node = current_node->data;
  }
  return rand_markov_node;
}

// full documentation in header file
MarkovNode *get_next_random_node (MarkovNode *state_struct_ptr)
{
  int rand_num = get_random_number (state_struct_ptr->total_word_count);
  int following_word = FIRST; //counts which word we are on
  NextNodeCounter *current_markov_node = state_struct_ptr->\
                                          counter_list[following_word];
  int times_counted = DEFAULT_TIMES_COUNTED; //times counted current word
  for (int i = 0; i < rand_num; i++)
  {
    if (times_counted == current_markov_node->frequency) //moves to next word
    {
      following_word++; //move to next word
      current_markov_node = state_struct_ptr->counter_list[following_word];
      times_counted = DEFAULT_TIMES_COUNTED; //restart to 1
    }
    else //counts word again until reaching frequency
    {
      times_counted++; //counted word again
    }
  }
  return current_markov_node->markov_node;
}

// full documentation in header file
void generate_random_sequence (MarkovChain *markov_chain, MarkovNode *
first_node, int max_length)
{
  if (!first_node)
  {
    first_node = get_first_random_node (markov_chain);
  }
  int sentence_len = 0;
  MarkovNode *current_node = first_node;
  while (sentence_len < max_length && current_node->counter_list != NULL)
  {
    markov_chain->print_func (current_node->data);
    if (current_node->counter_list != NULL)
    {
      current_node = get_next_random_node (current_node);
    }
    sentence_len++;
  }
  markov_chain->print_func (current_node->data);
}

// full documentation in header file
void free_markov_chain (MarkovChain **ptr_chain)
{
  int chain_len = (*ptr_chain)->database->size;
  Node *current_node = (*ptr_chain)->database->first;
  for (int i = 0; i < chain_len; i++)
  {
    if (current_node->data->counter_list)
    {
      for (int j = 0; j < current_node->data->following_words_count; j++)
      {
        if(current_node->data->counter_list[j])
        {
          free (current_node->data->counter_list[j]); //NextNodeCounter
          current_node->data->counter_list[j] = NULL;
        }
      }
      if(current_node->data->counter_list)
      {
        free (current_node->data->counter_list); //array of NextNodeCounter
        current_node->data->counter_list = NULL;

      }
    }
    (*ptr_chain)->free_data (current_node->data->data); //markov node data
    if (current_node->data)
    {
      free (current_node->data); //MarkovNode
      current_node->data = NULL;
    }
    if (current_node->next)
    {
      Node *prev_node = current_node;
      current_node = current_node->next;
      free (prev_node); //Node
      prev_node = NULL;
    }
  }
  if (current_node)
  {
    free (current_node); // last Node
    current_node = NULL;
  }
  (*ptr_chain)->database = NULL;
  *ptr_chain = NULL;
}

// full documentation in header file
bool add_node_to_counter_list (MarkovNode *first_node,
                               MarkovNode *second_node,
                               MarkovChain *markov_chain)
{
  if (markov_chain->is_last (first_node->data))
  {
    first_node->total_word_count++;
    return true;
  }
  for (int i = 0; i < first_node->following_words_count; i++)
  {
    if (markov_chain->comp_func (second_node->data,
                                 first_node->counter_list[i]->
                                     markov_node->data) == 0)
    {
      first_node->counter_list[i]->frequency++;
      first_node->total_word_count++;
      return true;
    }
  }
  first_node->counter_list = realloc (first_node->counter_list,
                                      (first_node->following_words_count + 1) *
                                      sizeof (NextNodeCounter));
  if (!first_node->counter_list)
  {
    return false;
  }
  NextNodeCounter *new_node = malloc (sizeof (NextNodeCounter));
  if (!new_node)
  {
    return false;
  }
  new_node->markov_node = second_node; //new following node
  new_node->frequency = 1;
  first_node->counter_list[first_node->following_words_count] = new_node;
  first_node->following_words_count++;
  first_node->total_word_count++;
  return true;
}

// full documentation in header file
Node *get_node_from_database (MarkovChain *markov_chain, void *data_ptr)
{
  Node *current_node = markov_chain->database->first;
  for (int i = 0; i < markov_chain->database->size; i++)
  {
    if (markov_chain->comp_func (current_node->data->data, data_ptr) == 0)
    {
      return current_node; //found node
    }
    if (current_node->next)
    {
      current_node = current_node->next;
    }
  }
  return NULL; //didn't find node
}

/**
 * initializes new markov node and adds to chain
 * @param markov_chain
 * @param new_markov_node
 * @param data_cpy
 */
void init_new_markov_node (MarkovChain *markov_chain,
                           MarkovNode *new_markov_node, void *data_cpy)
{
  new_markov_node->data = data_cpy;
  new_markov_node->counter_list = NULL;
  new_markov_node->total_word_count = 0;
  new_markov_node->following_words_count = 0;
  add (markov_chain->database, new_markov_node);
}

// full documentation in header file
Node *add_to_database (MarkovChain *markov_chain, void *data_ptr)
{
  Node *data_node = get_node_from_database (markov_chain, data_ptr);
  if (data_node)
  {
    return data_node; //if node exists already
  }
  MarkovNode *new_markov_node = malloc (sizeof (MarkovNode)); //new node
  if (!new_markov_node)
  {
    return NULL;
  }
  void *data_cpy = markov_chain->copy_func (data_ptr); //copy data to copy
  if (!data_cpy)
  {
    return NULL;
  }
  init_new_markov_node (markov_chain, new_markov_node, data_cpy); //add node
  return markov_chain->database->last; //return new node in linked list
}
