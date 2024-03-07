#include "PQueue.hpp"
#include "CubeDefines.hpp"

//NOTES FOR MYSELF: Use these functions
//  void cube_print(const char* format, ...);
//  void cube_assert_debug(bool condition, const char* file, uint16_t line, const char* str = nullptr, ...);

void test_pqueue_send_receive() {
    PQueue<int, 5> pQueue;
    
    CUBE_PRINT("Testing PQueue Send and Receive\n");

    //Send items to the queue
    for (int i = 0; i < 5; ++i) {
        bool sent = pQueue.Send(i, Priority::NORMAL);
        // Assert that sending was successful
        CUBE_ASSERT(sent, "Sending to PQueue failed");
    }

    //Receive items from the queue and checks if data is correct
    for (int i = 0; i < 5; ++i) {
        int received;
        bool receivedSuccessfully = pQueue.Receive(received);
        CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
        CUBE_ASSERT(received == i, "Received incorrect item from PQueue"); 
    }
}

//Tests if the PQueue adds another item if its full
void test_pqueue_full() {
    PQueue<int, 2> pQueue;

    CUBE_PRINT("Testing PQueue Full\n");

    //Fill up the queue
    pQueue.Send(1, Priority::NORMAL);
    pQueue.Send(2, Priority::NORMAL);

    bool sent = pQueue.Send(3, Priority::NORMAL);
    CUBE_ASSERT(!sent, "Sending to full PQueue succeeded");
}

//Test to receive an item from a empty Queue
void test_pqueue_empty() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue Empty\n");

    int received;
    bool receivedSuccessfully = pQueue.Receive(received);
    CUBE_ASSERT(!receivedSuccessfully, "Receiving from empty PQueue succeeded");
}

//Test if first item recieved from queue is highest priority  
void test_pqueue_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue Priority\n");

    pQueue.Send(1, Priority::LOW);
    pQueue.Send(2, Priority::HIGH);
    pQueue.Send(3, Priority::NORMAL);

    int received;
    bool receivedSuccessfully = pQueue.Receive(received);
    CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
    CUBE_ASSERT(received == 2, "Received incorrect item from PQueue");
}

void test_pqueue_receive_wait() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue ReceiveWait\n");

    // Send an item to the queue
    pQueue.Send(1, Priority::NORMAL);

    // Receive an item from the queue using ReceiveWait
    int received;
    bool receivedSuccessfully = pQueue.ReceiveWait(received);
    CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
    // Check if received item is correct
    CUBE_ASSERT(received == 1, "Received incorrect item from PQueue");
}

void test_pqueue_current_count() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue GetCurrentCount\n");

    // Send items to the queue
    for (int i = 0; i < 3; ++i) {
        pQueue.Send(i, Priority::NORMAL);
    }

    // Check the current count of the queue
    uint16_t count = pQueue.GetCurrentCount();
    CUBE_ASSERT(count == 3, "Incorrect count in PQueue");
}

void test_pqueue_max_depth() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue GetMaxDepth\n");

    // Check the max depth of the queue
    uint16_t maxDepth = pQueue.GetMaxDepth();
    CUBE_ASSERT(maxDepth == 5, "Incorrect max depth in PQueue");
}

void test_pqueue_send_high_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue Send with High Priority\n");

    // Send an item to the queue with high priority
    bool sent = pQueue.Send(1, Priority::HIGH);
    CUBE_ASSERT(sent, "Sending to PQueue with high priority failed");
}

void test_pqueue_send_low_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue Send with Low Priority\n");

    // Send an item to the queue with low priority
    bool sent = pQueue.Send(1, Priority::LOW);
    CUBE_ASSERT(sent, "Sending to PQueue with low priority failed");
}

void test_pqueue_receive_high_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue Receive with High Priority\n");

    // Send an item to the queue with high priority
    pQueue.Send(1, Priority::HIGH);

    // Receive an item from the queue
    int received;
    bool receivedSuccessfully = pQueue.Receive(received);
    CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
    // Check if received item is correct
    CUBE_ASSERT(received == 1, "Received incorrect item from PQueue");
}

void test_pqueue_receive_low_priority() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue Receive with Low Priority\n");

    // Send an item to the queue with low priority
    pQueue.Send(1, Priority::LOW);

    // Receive an item from the queue
    int received;
    bool receivedSuccessfully = pQueue.Receive(received);
    CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
    // Check if received item is correct
    CUBE_ASSERT(received == 1, "Received incorrect item from PQueue");
}

void test_pqueue_priority_ordering() {
    PQueue<int, 5> pQueue;

    CUBE_PRINT("Testing PQueue Priority Ordering\n");

    // Send items to the queue with different priorities
    pQueue.Send(1, Priority::LOW);
    pQueue.Send(2, Priority::HIGH);
    pQueue.Send(3, Priority::NORMAL);

    // Receive items from the queue and ensure they are received in priority order
    int received;
    bool receivedSuccessfully;

    // Receive high priority item first
    receivedSuccessfully = pQueue.Receive(received);
    CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
    CUBE_ASSERT(received == 2, "Received incorrect item from PQueue");

    // Receive normal priority item next
    receivedSuccessfully = pQueue.Receive(received);
    CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
    CUBE_ASSERT(received == 3, "Received incorrect item from PQueue");

    // Receive low priority item last
    receivedSuccessfully = pQueue.Receive(received);
    CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
    CUBE_ASSERT(received == 1, "Received incorrect item from PQueue");
}

void test_pqueue_max_depth_exceeded() {
    PQueue<int, 3> pQueue;

    CUBE_PRINT("Testing PQueue Maximum Depth Exceeded\n");

    // Fill up the queue to its maximum depth
    for (int i = 0; i < 3; ++i) {
        bool sent = pQueue.Send(i, Priority::NORMAL);
        CUBE_ASSERT(sent, "Sending to PQueue failed");
    }

    // Attempt to send another item, which should fail
    bool sent = pQueue.Send(4, Priority::NORMAL);
    CUBE_ASSERT(!sent, "Sending to full PQueue succeeded");

    // Ensure that the queue is still full
    CUBE_ASSERT(pQueue.IsFull(), "PQueue is not full when it should be");
}

void test_pqueue_concurrent_access() {
    constexpr int NumIterations = 1000; // Number of iterations per thread

    // Create a PQueue instance
    PQueue<int, 10> pQueue;

    // Simulate concurrent access using alternating send and receive operations
    for (int i = 0; i < NumIterations; ++i) {
        // Alternate between sending and receiving items
        if (i % 2 == 0) {
            // Send item with iteration number as data
            pQueue.Send(i, Priority::NORMAL);
        } else {
            // Receive item
            int received;
            bool receivedSuccessfully = pQueue.Receive(received);
            // Assert that receiving was successful
            CUBE_ASSERT(receivedSuccessfully, "Receiving from PQueue failed");
            // Assert that the received item matches the iteration number
            CUBE_ASSERT(received == i, "Received incorrect item from PQueue");
        }
    }

    // Ensure the queue is empty after all iterations
    CUBE_ASSERT(pQueue.IsEmpty(), "PQueue is not empty after concurrent access");
}


int main() {
    test_pqueue_send_receive();
    test_pqueue_full();
    test_pqueue_empty();
    test_pqueue_priority();
    test_pqueue_receive_wait();
    test_pqueue_current_count();
    test_pqueue_max_depth();
    test_pqueue_send_high_priority();
    test_pqueue_send_low_priority();
    test_pqueue_receive_high_priority();
    test_pqueue_receive_low_priority();
    test_pqueue_priority_ordering();
    test_pqueue_max_depth_exceeded();
    test_pqueue_concurrent_access();

    return 0;
}
