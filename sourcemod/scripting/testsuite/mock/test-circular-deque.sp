#pragma semicolon 1
#pragma newdecls required

#if !defined DEBUG
    #define  DEBUG
#endif

#if !defined _DEBUG
    #define  _DEBUG
#endif

#if defined NDEBUG
    #undef  NDEBUG
#endif

#include <sourcemod>
#include <log4sp/details/circular_deque>

#include "../assert"
#include "../test_utils"


public void OnPluginStart()
{
    Test();
    RegServerCmd("sm_log4sp_test_circular_deque", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("---------- Started testing CircularDeque ----------");

    TestSize();

    TestRolling();

    TestEmpty();

    TestPushFront();

    TestPopBack();

    TestString();

    TestBlockSize();

    TestCloneEquals();

    PrintToServer("----------- Test CircularDeque ended -------------");
}

void TestSize()
{
    SetTestContext("CircularDeque Size");

    const int MAX_SIZE = 4;
    CircularDeque q = new CircularDeque(MAX_SIZE);

    AssertEq("[initial length]", q.Length, 0);
    AssertTrue("[initial empty]", q.IsEmpty());
    AssertFalse("[initial full]", q.IsFull());
    AssertEq("[max size]", q.GetMaxSize(), MAX_SIZE);
    AssertEq("[default block size]", q.BlockSize, 1);

    for (int i = 0; i < MAX_SIZE; ++i)
        q.PushBack(i);

    AssertEq("[length]", q.Length, MAX_SIZE);
    AssertFalse("[empty]", q.IsEmpty());
    AssertTrue("[full]", q.IsFull());

    // Overrun: the oldest (front) is overwritten and the head advances.
    q.PushBack(999);
    AssertEq("[length after overrun]", q.Length, MAX_SIZE);
    AssertEq("[front after overrun]", q.GetFront(), 1);
    AssertEq("[back after overrun]", q.GetBack(), 999);

    q.Close();
}

void TestRolling()
{
    SetTestContext("CircularDeque Rolling");

    const int Q_SIZE = 4;
    CircularDeque q = new CircularDeque(Q_SIZE);

    // Push more than the queue capacity (0, 1, 2, 3, 4, 5).
    for (int i = 0; i < Q_SIZE + 2; ++i)
        q.PushBack(i);

    AssertEq("[length]", q.Length, Q_SIZE);

    AssertEq("[front]", q.GetFront(), 2);
    q.PopFront();
    AssertEq("[front]", q.GetFront(), 3);
    q.PopFront();
    AssertEq("[front]", q.GetFront(), 4);
    q.PopFront();
    AssertEq("[front]", q.GetFront(), 5);
    q.PopFront();
    AssertTrue("[empty after pop]", q.IsEmpty());

    q.PushBack(6);
    AssertEq("[front after push]", q.GetFront(), 6);
    AssertEq("[back after push]", q.GetBack(), 6);

    q.Close();
}

void TestEmpty()
{
    SetTestContext("CircularDeque Empty");

    // A queue with max size 0 is disabled - pushes are ignored.
    CircularDeque q = new CircularDeque(0);

    AssertEq("[max size]", q.GetMaxSize(), 0);
    AssertTrue("[empty]", q.IsEmpty());
    AssertFalse("[full]", q.IsFull());

    q.PushBack(1);
    AssertTrue("[still empty after push back]", q.IsEmpty());
    AssertEq("[length]", q.Length, 0);

    q.PushFront(2);
    AssertTrue("[still empty after push front]", q.IsEmpty());

    q.Close();
}

void TestPushFront()
{
    SetTestContext("CircularDeque PushFront");

    CircularDeque q = new CircularDeque(4);

    q.PushFront(1);
    q.PushFront(2);
    q.PushFront(3);
    q.PushFront(4);
    AssertTrue("[full]", q.IsFull());
    AssertEq("[front]", q.GetFront(), 4);
    AssertEq("[back]", q.GetBack(), 1);

    // Overrun from the front: overwrites the back and the head moves backwards.
    q.PushFront(5);
    AssertEq("[length after push]", q.Length, 4);
    AssertEq("[front after push]", q.GetFront(), 5);
    AssertEq("[back after push]", q.GetBack(), 2);

    q.Close();
}

void TestPopBack()
{
    SetTestContext("CircularDeque PopBack");

    CircularDeque q = new CircularDeque(4);

    q.PushBack(1);
    q.PushBack(2);
    q.PushBack(3);
    q.PushBack(4);
    AssertTrue("[full]", q.IsFull());

    q.PopBack();
    AssertEq("[length after pop]", q.Length, 3);
    AssertEq("[back after pop]", q.GetBack(), 3);
    AssertEq("[front after pop]", q.GetFront(), 1);

    q.PopBack();
    AssertEq("[length]", q.Length, 2);
    AssertEq("[back]", q.GetBack(), 2);

    q.Close();
}

void TestString()
{
    SetTestContext("CircularDeque String");

    CircularDeque q = new CircularDeque(2, 128);

    q.PushBackString("alpha");
    q.PushBackString("beta");
    AssertTrue("[full]", q.IsFull());

    char buffer[128];
    q.GetFrontString(buffer, sizeof(buffer));
    AssertStrEq("[front string]", buffer, "alpha");
    q.GetBackString(buffer, sizeof(buffer));
    AssertStrEq("[back string]", buffer, "beta");

    // Overrun from the front: overwrites the back and the head moves backwards.
    q.PushFrontString("gamma");
    q.GetFrontString(buffer, sizeof(buffer));
    AssertStrEq("[front after push]", buffer, "gamma");
    q.GetBackString(buffer, sizeof(buffer));
    AssertStrEq("[back after push]", buffer, "alpha");

    q.Close();
}

void TestBlockSize()
{
    SetTestContext("CircularDeque BlockSize");

    CircularDeque q = new CircularDeque(2, 3);

    AssertEq("[block size]", q.BlockSize, 3);
    AssertEq("[max size]", q.GetMaxSize(), 2);

    int e0[3] = {1, 2, 3};
    int e1[3] = {4, 5, 6};
    q.PushBackArray(e0, 3);
    q.PushBackArray(e1, 3);

    int buffer[3];
    q.GetFrontArray(buffer, 3);
    AssertArrayEq("[front array]", buffer, e0, 3);
    q.GetBackArray(buffer, 3);
    AssertArrayEq("[back array]", buffer, e1, 3);

    // Overrun from the front: overwrites the back and the head moves backwards.
    int e2[3] = {7, 8, 9};
    q.PushFrontArray(e2, 3);
    q.GetFrontArray(buffer, 3);
    AssertArrayEq("[front array after push]", buffer, e2, 3);
    q.GetBackArray(buffer, 3);
    AssertArrayEq("[back array after push]", buffer, e0, 3);

    q.Close();
}

void TestCloneEquals()
{
    SetTestContext("CircularDeque Clone & Equals");

    CircularDeque q = new CircularDeque(4);
    q.PushBack(1);
    q.PushBack(2);
    q.PushBack(3);

    // Clone shares the same underlying storage with the original.
    CircularDeque clone = q.Clone();

    AssertTrue("[clone equals q]", clone.Equals(q));
    AssertTrue("[q equals clone]", q.Equals(clone));
    AssertEq("[clone length]", clone.Length, 3);
    AssertEq("[clone front]", clone.GetFront(), 1);
    AssertEq("[clone back]", clone.GetBack(), 3);

    // The shared storage must be closed exactly once per reference.
    clone.Close();
    q.Close();
}
