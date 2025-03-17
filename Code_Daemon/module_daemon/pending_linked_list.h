#ifndef PENDING_LINKED_LIST_H
#define PENDING_LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include "general.h"



void insert_head(linked_list_t **head, const char *ip, const unsigned char *mac, int count2, int status);
void insert_tail(linked_list_t **head, const char *ip, const unsigned char *mac, int count2, int status);
void insert_any_pos(linked_list_t **head, int pos, const char *ip, const unsigned char *mac, int count2, int status);

void delete_head(linked_list_t **head);
void delete_tail(linked_list_t **head);
void delete_any_pos(linked_list_t **head, int pos);
void delete_to_value(linked_list_t **head, const char *ip);

void update_to_value(linked_list_t **head, uint8_t *ip, uint8_t *mac);

#endif 