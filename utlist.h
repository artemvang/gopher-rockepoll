/*
Copyright (c) 2007-2018, Troy D. Hanson   http://troydhanson.github.com/uthash/
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UTLIST_H
#define UTLIST_H

#define UTLIST_VERSION 2.1.0


#define LL_APPEND(head,add)                                                   \
do {                                                                          \
    if (head) {                                                               \
        (add)->next = head;     /* use add->next as a temp variable */        \
        while ((add)->next->next) { (add)->next = (add)->next->next; }        \
        (add)->next->next=(add);                                              \
    } else {                                                                  \
        (head)=(add);                                                         \
    }                                                                         \
    (add)->next=NULL;                                                         \
} while (0)


#define LL_DELETE(head,del)                                                   \
do {                                                                          \
    __typeof(head) _tmp;                                                      \
    if ((head) == (del)) {                                                    \
        (head)=(head)->next;                                                  \
    } else {                                                                  \
        _tmp = (head);                                                        \
        while (_tmp->next && (_tmp->next != (del))) {                         \
            _tmp = _tmp->next;                                                \
        }                                                                     \
        if (_tmp->next) {                                                     \
            _tmp->next = (del)->next;                                         \
        }                                                                     \
    }                                                                         \
} while (0)


#define LL_FOREACH_SAFE(head,el,tmp)                                          \
    for ((el) = (head); (el) && ((tmp) = (el)->next, 1); (el) = (tmp))


#define DL_APPEND(head,add)                                                   \
do {                                                                          \
  if (head) {                                                                 \
      (add)->prev = (head)->prev;                                             \
      (head)->prev->next = (add);                                             \
      (head)->prev = (add);                                                   \
      (add)->next = NULL;                                                     \
  } else {                                                                    \
      (head)=(add);                                                           \
      (head)->prev = (head);                                                  \
      (head)->next = NULL;                                                    \
  }                                                                           \
} while (0)


#define DL_DELETE(head,del)                                                   \
do {                                                                          \
    if ((del)->prev == (del)) {                                               \
        (head)=NULL;                                                          \
    } else if ((del)==(head)) {                                               \
        (del)->next->prev = (del)->prev;                                      \
        (head) = (del)->next;                                                 \
    } else {                                                                  \
        (del)->prev->next = (del)->next;                                      \
        if ((del)->next) {                                                    \
            (del)->next->prev = (del)->prev;                                  \
        } else {                                                              \
            (head)->prev = (del)->prev;                                       \
        }                                                                     \
    }                                                                         \
} while (0)


#define DL_FOREACH_SAFE(head,el,tmp)                                          \
    for ((el) = (head); (el) && ((tmp) = (el)->next, 1); (el) = (tmp))

#endif