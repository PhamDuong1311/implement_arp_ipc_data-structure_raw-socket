#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pending_linked_list.h"
#include "arp_cache.h"

void insert_head(linked_list_t **head, const char *ip, const unsigned char *mac, int count2, int status) {
    linked_list_t *new_node = (linked_list_t *) malloc(sizeof(linked_list_t));
    strncpy(new_node->data.ip, ip, 16);
    memcpy(new_node->data.mac, mac, 6);
    new_node->data.timestamp = time(NULL);
    new_node->data.count2 = count2;
    new_node->data.status = status;
    
    new_node->next = *head;
    *head = new_node;
}

void insert_tail(linked_list_t **head, const char *ip, const unsigned char *mac, int count2, int status) {
    linked_list_t *new_node = (linked_list_t *) malloc(sizeof(linked_list_t));
    strncpy(new_node->data.ip, ip, 16);
    memcpy(new_node->data.mac, mac, 6);
    new_node->data.timestamp = time(NULL);
    new_node->data.count2 = count2;
    new_node->data.status = status;

    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
        return;
    }
    
    linked_list_t *temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }

    temp->next = new_node;

}

void insert_any_pos(linked_list_t **head, int pos, const char *ip, const unsigned char *mac, int count2, int status) {
    if (pos == 1) {
        insert_head(head, value);
        return;
    }

    linked_list_t *new_node = (linked_list_t *) malloc(sizeof(linked_list_t));
    strncpy(new_node->data.ip, ip, 16);
    memcpy(new_node->data.mac, mac, 6);
    new_node->data.timestamp = time(NULL);
    new_node->data.count2 = count2;
    new_node->data.status = status;

    linked_list_t *temp = *head;
    for (int i = 1; (i <= pos - 1) && (temp->next != NULL); i++) {
        temp = temp->next;
    }

    new_node->next = temp->next;
    temp->next = new_node;
}

void delete_head(linked_list_t **head) {
    if (*head == NULL) return;

    linked_list_t *temp = *head;
    *head = (*head)->next;
    free(temp);
}

void delete_tail(linked_list_t **head) {
    if (*head == NULL) return;
    if ((*head)->next == NULL) {
        free(*head);
        *head = NULL;
        return;
    }

    linked_list_t *temp = *head;
    while (temp->next->next != NULL) {
        temp = temp->next;
    }

    linked_list_t *delete_node = temp->next;
    temp->next = NULL;
    free(delete_node);

}

void delete_any_pos(linked_list_t **head, int pos) {
    if (*head == NULL || pos < 1) return;
    if (pos == 1) {
        delete_head(head);
        return;
    }

    linked_list_t *temp = *head;
    for (int i = 1; (i <= pos - 1) && (temp->next != NULL); i++) {
        temp = temp->next;
    }

    if (temp == NULL || temp->next == NULL) return;

    linked_list_t *delete_node = temp->next;
    temp->next = temp->next->next;
    free(delete_node);
}

void delete_to_value(linked_list_t **head, const char *ip) {
    if (*head == NULL) return;

    if (strcmp((*head)->data.ip, ip) == 0) { 
        delete_head(head);
        return;
    }

    linked_list_t *temp = *head;
    while((temp->next != NULL) && (temp->next->data.ip != ip)) {
        temp = temp->next;
    }

    if (temp->next == NULL) return;

    linked_list_t *delete_node = temp->next;
    temp->next = temp->next->next;
    free(delete_node);
}

void update_to_value(linked_list_t **head, uint8_t *ip, uint8_t *mac) {
    linked_list_t *temp = *head;
    while((temp != NULL) && (temp->data.ip != ip)) {
        temp = temp->next;
    }


    if (temp != NULL) { 
        temp->data.mac = mac;
    }
}
