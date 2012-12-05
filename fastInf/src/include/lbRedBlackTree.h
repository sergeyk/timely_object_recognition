/* Copyright 2009 Ariel Jaimovich, Ofer Meshi, Ian McGraw and Gal Elidan */


/*
This file is part of FastInf library.

FastInf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FastInf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FastInf.  If not, see <http://www.gnu.org/licenses/>.
*/

//---------------------------------------------------------------------------
#ifndef TStorageHPP
#define TStorageHPP

//---------------------------------------------------------------------------
// Template implementation
// Red-Black Tree with Doubly Linked List
//
// Author:  Arjan van den Boogaard (arjan@threelittlewitches.nl)
//          Three Little Witches (www.threelittlewitches.nl)
//          7 feb 2004
//
// This is free software and you may use it as you wish, at your own risk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <stdlib.h>
#include <assert.h>

//---------------------------------------------------------------------------
template <class K, class D> class TStorage;
template <class K, class D> class TStorageNode;

//---------------------------------------------------------------------------
enum TStorageColor { scRED, scBLACK, scDUP };

//---------------------------------------------------------------------------
template <class K, class D> class TStorageNode
{
    public:
        // Returns the key. The key is read-only but can be changed by
        // using the ReplaceKey method in the storage object
        K GetKey() const                    { return Key;  }

        // Get or set the user data
        D       GetData() const             { return Data; }
        void    SetData(const D &newdata)   { Data = newdata; }

        // Get the previous or next node in the storage. This ordered
        // traversal includes the duplicates. For only the unique nodes use
        // the GetNextUnique and GetPrevUnique methods in the storage object
        TStorageNode<K,D> *  GetNext()      { return Next; }
        TStorageNode<K,D> *  GetPrev()      { return Prev; }
        TStorageNode<K,D> const *  GetNextConst() const { return Next; }
        TStorageNode<K,D> const *  GetPrevConst() const { return Prev; }

    private:
        friend class TStorage<K,D>;

        TStorageNode<K,D> * Next;
        TStorageNode<K,D> * Prev;
        TStorageNode<K,D> * Left;
        TStorageNode<K,D> * Right;
        TStorageNode<K,D> * Parent;
        TStorageColor       Color;

        K   Key;
        D   Data;
};

//---------------------------------------------------------------------------
template <class K, class D> class TStorage
{
    public:
        TStorage();

        // Deleting a storage object clears all remaining nodes
        virtual ~TStorage();

        // Add and Insert are the same (for backwards compatibility :)
        TStorageNode<K,D> * Insert(const K &key, const D &data);
        TStorageNode<K,D> * Add(const K &key, const D &data);

        // Deletes the specified node from the tree. The pointer is no
        // longer valid after the delete
        void Delete(TStorageNode<K,D> * node);

        // Find the first instance of the specified value
        TStorageNode<K,D> * Find(const K &key);

        // Replaces the key (and thus change sorting order)
        void  ReplaceKey(TStorageNode<K,D> * node, const K &newkey);

        // Clear wipes the complete tree
        void Clear();

        // First and Last are the head and tail of the doubly linked list
        TStorageNode<K,D> * GetFirst();
        TStorageNode<K,D> * GetLast();

        TStorageNode<K,D> const * GetFirstConst() const { return First; }
        TStorageNode<K,D> const * GetLastConst() const { return Last; }

        // Returns the total amount of nodes in the storage
        int GetCount();

        // Traversal through all unique nodes
        TStorageNode<K,D> * GetNextUnique(TStorageNode<K,D> * node);
        TStorageNode<K,D> * GetPrevUnique(TStorageNode<K,D> * node);

    protected:
        // Use a seperate compare function to allow overloaded compare
        // Needs to return < 0 is one smaller than two, > 0 is larger,
        // 0 if equal.
        virtual int Compare(const K &one, const K&two);

    private:
        TStorageNode<K,D> *  First;
        TStorageNode<K,D> *  Last;
        TStorageNode<K,D> *  Root;
        TStorageNode<K,D> *  NIL;
        int                  mCount;

        void RotateRight(TStorageNode<K,D> * x);
        void RotateLeft(TStorageNode<K,D> * x);
        void FixInsert(TStorageNode<K,D> * x);
        void FixDelete(TStorageNode<K,D> * x);
        void DoInsert(TStorageNode<K,D> * x);
        void DoDelete(TStorageNode<K,D> * x);

        void DLLDelete(TStorageNode<K,D> * x);
        void DLLInsertBefore(TStorageNode<K,D> * x, TStorageNode<K,D> * y);
        void DLLAddAfter(TStorageNode<K,D> * x, TStorageNode<K,D> * y);

        TStorageNode<K,D> *  TreeNext(TStorageNode<K,D> * x);
        TStorageNode<K,D> *  TreePrev(TStorageNode<K,D> * x);
        TStorageNode<K,D> *  FindMin(TStorageNode<K,D> * x);
        TStorageNode<K,D> *  FindMax(TStorageNode<K,D> * x);

        void InitNode(TStorageNode<K,D> * x);
        void ReplaceTreeNode(TStorageNode<K,D> * x, TStorageNode<K,D> * y);
};

//---------------------------------------------------------------------------
template <class K, class D>
TStorage<K,D>::TStorage()
{
    NIL = new TStorageNode<K,D>;
    InitNode(NIL);
    NIL->Color = scBLACK;

    Root = NIL;
    First = NULL;
    Last = NULL;
    mCount = 0;
}

//---------------------------------------------------------------------------
template <class K, class D>
TStorage<K,D>::~TStorage()
{
    Clear();
    delete NIL;
}

//---------------------------------------------------------------------------
template <class K, class D>
TStorageNode<K,D> *  TStorage<K,D>::Add(const K &key, const D &data)
{
    return Insert(key, data);
}

//---------------------------------------------------------------------------
template <class K, class D>
TStorageNode<K,D> *  TStorage<K,D>::Insert(const K &key, const D &data)
{
    TStorageNode<K,D> *  node;

    node = new TStorageNode<K,D>;
    InitNode(node);

    node->Key = key;
    node->Data = data;

    DoInsert(node);

    mCount++;

    return node;
}

//---------------------------------------------------------------------------
template <class K, class D>
void TStorage<K,D>::Delete(TStorageNode<K,D> * node)
{
    assert(node);

    DoDelete(node);

    mCount--;

    delete node;
}

//---------------------------------------------------------------------------
template <class K, class D>
TStorageNode<K,D> *  TStorage<K,D>::Find(const K &key)
{
    TStorageNode<K,D> *  y = Root;
    int c;

    while (y != NIL)
    {
        c = Compare(key, y->Key);
        if (c == 0) return y;
        if (c < 0)  y = y->Left;
        else        y = y->Right;
    }

    return NULL;
}

//---------------------------------------------------------------------------
template <class K, class D>
void TStorage<K,D>::ReplaceKey(TStorageNode<K,D> * node, const K &newkey)
{
    assert(node);

    DoDelete(node);
    InitNode(node);

    node->Key = newkey;

    DoInsert(node);
}

//---------------------------------------------------------------------------
template <class K, class D>
void            TStorage<K,D>::Clear()
{
    TStorageNode<K,D> *  x = First;
    TStorageNode<K,D> *  y;

    while (x)
    {
        y = x;
        x = x->Next;
        delete y;
    }

    First = NULL;
    Last = NULL;
    Root = NIL;
    mCount = 0;
}

//---------------------------------------------------------------------------
template <class K, class D>
TStorageNode<K,D> *  TStorage<K,D>::GetFirst()
{
    return First;
}

//---------------------------------------------------------------------------
template <class K, class D>
TStorageNode<K,D> *  TStorage<K,D>::GetLast()
{
    return Last;
}

//---------------------------------------------------------------------------
template <class K, class D>
int             TStorage<K,D>::GetCount()
{
    return mCount;
}

//---------------------------------------------------------------------------
template <class K, class D>
TStorageNode<K,D> *  TStorage<K,D>::GetNextUnique(TStorageNode<K,D> * node)
{
    assert(node);

    if (node->Color == scDUP) node = Find(node->Key);
    if (node) node = TreeNext(node);

    return node;
}

//---------------------------------------------------------------------------
template <class K, class D>
TStorageNode<K,D> *  TStorage<K,D>::GetPrevUnique(TStorageNode<K,D> * node)
{
    assert(node);

    if (node->Color == scDUP) node = Find(node->Key);
    if (node) node = TreePrev(node);

    return node;
}

//---------------------------------------------------------------------------
template <class K, class D>
int TStorage<K,D>::Compare(const K &one, const K &two)
{
    if (one < two) return -1;
    if (one > two) return 1;
    return 0;
}

//---------------------------------------------------------------------------
template <class K, class D>
void            TStorage<K,D>::RotateRight(TStorageNode<K,D> * x)
{
    TStorageNode<K,D> * y = x->Left;

    x->Left = y->Right;
    if (y->Right != NIL) y->Right->Parent = x;

    if (y != NIL) y->Parent = x->Parent;
    if (x->Parent)
    {
        if (x == x->Parent->Right)  x->Parent->Right = y;
        else                        x->Parent->Left = y;
    }
    else
    {
        Root = y;
    }

    y->Right = x;
    if (x != NIL) x->Parent = y;
}

//---------------------------------------------------------------------------
template <class K, class D>
void            TStorage<K,D>::RotateLeft(TStorageNode<K,D> * x)
{
    TStorageNode<K,D> * y = x->Right;

    x->Right = y->Left;
    if (y->Left != NIL) y->Left->Parent = x;

    if (y != NIL) y->Parent = x->Parent;
    if (x->Parent)
    {
        if (x == x->Parent->Left)   x->Parent->Left = y;
        else                        x->Parent->Right = y;
    }
    else
    {
        Root = y;
    }

    y->Left = x;
    if (x != NIL) x->Parent = y;
}

//---------------------------------------------------------------------------
template <class K, class D>
void            TStorage<K,D>::FixInsert(TStorageNode<K,D> * x)
{
    while (x != Root && x->Parent->Color == scRED)
    {
        if (x->Parent == x->Parent->Parent->Left)
        {
            TStorageNode<K,D> *  y = x->Parent->Parent->Right;
            if (y->Color == scRED)
            {
                x->Parent->Color = scBLACK;
                y->Color = scBLACK;
                x->Parent->Parent->Color = scRED;
                x = x->Parent->Parent;
            }
            else
            {
                if (x == x->Parent->Right)
                {
                    x = x->Parent;
                    RotateLeft(x);
                }

                x->Parent->Color = scBLACK;
                x->Parent->Parent->Color = scRED;
                RotateRight(x->Parent->Parent);
            }
        }
        else
        {
            TStorageNode<K,D> *  y = x->Parent->Parent->Left;
            if (y->Color == scRED)
            {
                x->Parent->Color = scBLACK;
                y->Color = scBLACK;
                x->Parent->Parent->Color = scRED;
                x = x->Parent->Parent;
            }
            else
            {
                if (x == x->Parent->Left)
                {
                    x = x->Parent;
                    RotateRight(x);
                }
                x->Parent->Color = scBLACK;
                x->Parent->Parent->Color = scRED;
                RotateLeft(x->Parent->Parent);
            }
        }
    }

    Root->Color = scBLACK;
}

//---------------------------------------------------------------------------
template <class K, class D>
void            TStorage<K,D>::DoInsert(TStorageNode<K,D> * x)
{
    TStorageNode<K,D> *  y;
    TStorageNode<K,D> *  p;
    int c = 0;

    y = Root;
    p = NULL;

    while (y != NIL)
    {
        c = Compare(x->Key, y->Key);
        if (c == 0)
        {
            x->Color = scDUP;
            y = TreeNext(y);
            if (y)  DLLInsertBefore(y, x);
            else    DLLAddAfter(Last, x);

            return;
        }

        p = y;
        y = (c < 0) ? y->Left : y->Right;
    }

    if (p)
    {
        x->Parent = p;

        if (c < 0)
        {
            DLLInsertBefore(p, x);

            p->Left = x;
        }
        else
        {
            y = TreeNext(p);
            if (y)  DLLInsertBefore(y, x);
            else    DLLAddAfter(Last, x);

            p->Right = x;
        }
    }
    else
    {
        Root = x;
        x->Color = scBLACK;
        First = x;
        Last = x;
    }

    FixInsert(x);
}

//---------------------------------------------------------------------------
template <class K, class D>
void            TStorage<K,D>::FixDelete(TStorageNode<K,D> * x)
{
    while (x != Root && x->Color == scBLACK)
    {
        if (x == x->Parent->Left)
        {
            TStorageNode<K,D> * w = x->Parent->Right;
            if (w->Color == scRED)
            {
                w->Color = scBLACK;
                x->Parent->Color = scRED;
                RotateLeft (x->Parent);
                w = x->Parent->Right;
            }
            if (w->Left->Color == scBLACK && w->Right->Color == scBLACK)
            {
                w->Color = scRED;
                x = x->Parent;
            }
            else
            {
                if (w->Right->Color == scBLACK)
                {
                    w->Left->Color = scBLACK;
                    w->Color = scRED;
                    RotateRight(w);
                    w = x->Parent->Right;
                }
                w->Color = x->Parent->Color;
                x->Parent->Color = scBLACK;
                w->Right->Color = scBLACK;
                RotateLeft (x->Parent);
                x = Root;
            }
        }
        else
        {
            TStorageNode<K,D> * w = x->Parent->Left;
            if (w->Color == scRED) {
                w->Color = scBLACK;
                x->Parent->Color = scRED;
                RotateRight (x->Parent);
                w = x->Parent->Left;
            }
            if (w->Right->Color == scBLACK && w->Left->Color == scBLACK)
            {
                w->Color = scRED;
                x = x->Parent;
            }
            else
            {
                if (w->Left->Color == scBLACK)
                {
                    w->Right->Color = scBLACK;
                    w->Color = scRED;
                    RotateLeft (w);
                    w = x->Parent->Left;
                }
                w->Color = x->Parent->Color;
                x->Parent->Color = scBLACK;
                w->Left->Color = scBLACK;
                RotateRight (x->Parent);
                x = Root;
            }
        }
    }
    x->Color = scBLACK;
}

//---------------------------------------------------------------------------
template <class K, class D>
void            TStorage<K,D>::DoDelete(TStorageNode<K,D> * x)
{
    if (x->Color == scDUP)
    {
        DLLDelete(x);
        return;
    }

    if (x->Next && x->Next->Color == scDUP)
    {
        ReplaceTreeNode(x, x->Next);
        DLLDelete(x);
        return;
    }

    DLLDelete(x);

    TStorageNode<K,D> *  y;
    TStorageNode<K,D> *  z;
    bool            balance;

    if (x->Left == NIL || x->Right == NIL)  y = x;
    else                                    y = FindMin(x->Right);

    if (y->Left != NIL) z = y->Left;
    else                z = y->Right;

    z->Parent = y->Parent;
    if (y->Parent)
    {
        if (y == y->Parent->Left)   y->Parent->Left = z;
        else                        y->Parent->Right = z;
    }
    else
    {
        Root = z;
    }

    balance = (y->Color == scBLACK);

    if (y != x) ReplaceTreeNode(x, y);

    if (balance) FixDelete(z);
}

//---------------------------------------------------------------------------
template <class K, class D>
void            TStorage<K,D>::DLLDelete(TStorageNode<K,D> * x)
{
    if (x->Prev)    x->Prev->Next = x->Next;
    else            First = x->Next;
    if (x->Next)    x->Next->Prev = x->Prev;
    else            Last = x->Prev;
}

//---------------------------------------------------------------------------
template <class K, class D>
void            TStorage<K,D>::DLLInsertBefore(TStorageNode<K,D> * x, TStorageNode<K,D> * y)
{
    y->Prev = x->Prev;
    if (y->Prev)    y->Prev->Next = y;
    else            First = y;
    y->Next = x;
    x->Prev = y;
}

//---------------------------------------------------------------------------
template <class K, class D>
void            TStorage<K,D>::DLLAddAfter(TStorageNode<K,D> * x, TStorageNode<K,D> * y)
{
    y->Next = x->Next;
    if (y->Next)    y->Next->Prev = y;
    else            Last = y;
    y->Prev = x;
    x->Next = y;
}

//---------------------------------------------------------------------------
template <class K, class D>
void            TStorage<K,D>::InitNode(TStorageNode<K,D> * x)
{
    x->Next = NULL;
    x->Prev = NULL;
    x->Parent = NULL;
    x->Left = NIL;
    x->Right = NIL;
    x->Color = scRED;
}

//---------------------------------------------------------------------------
template <class K, class D>
TStorageNode<K,D> *  TStorage<K,D>::TreeNext(TStorageNode<K,D> * x)
{
    if (x->Right != NIL) return FindMin(x->Right);

    while (x->Parent)
    {
        if (x == x->Parent->Left) break;

        x = x->Parent;
    }

    return x->Parent;
}

//---------------------------------------------------------------------------
template <class K, class D>
TStorageNode<K,D> *  TStorage<K,D>::TreePrev(TStorageNode<K,D> * x)
{
    if (x->Left != NIL) return FindMax(x->Left);

    while (x->Parent)
    {
        if (x == x->Parent->Right) break;

        x = x->Parent;
    }

    return x->Parent;
}

//---------------------------------------------------------------------------
template <class K, class D>
TStorageNode<K,D> *  TStorage<K,D>::FindMin(TStorageNode<K,D> * x)
{
    while (x->Left != NIL) x = x->Left;
    return x;
}

//---------------------------------------------------------------------------
template <class K, class D>
TStorageNode<K,D> *  TStorage<K,D>::FindMax(TStorageNode<K,D> * x)
{
    while (x->Right != NIL) x = x->Right;
    return x;
}

//---------------------------------------------------------------------------
template <class K, class D>
void            TStorage<K,D>::ReplaceTreeNode(TStorageNode<K,D> * x, TStorageNode<K,D> * y)
{
    y->Color = x->Color;
    y->Left = x->Left;
    y->Right = x->Right;
    y->Parent = x->Parent;

    if (y->Parent)
    {
        if (y->Parent->Left == x)   y->Parent->Left = y;
        else                        y->Parent->Right = y;
    }
    else
    {
        Root = y;
    }

    y->Left->Parent = y;
    y->Right->Parent = y;
}

//---------------------------------------------------------------------------
#endif

