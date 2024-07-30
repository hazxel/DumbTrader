#ifndef DUMBTRADER_IPC_POSIX_SHARED_MEMORY_RING_BUFFER_H_
#define DUMBTRADER_IPC_POSIX_SHARED_MEMORY_RING_BUFFER_H_

#include "dumbtrader/ipc/posix_shared_memory.h"
#include "dumbtrader/ipc/posix_semaphore.h"

#include <memory>
#include <string>


namespace dumbtrader::ipc {

template<typename EleType>
struct SharedBuffer {
    size_t head;
    size_t tail;
    EleType data[];
};

template<typename EleType, bool IsOwner>
class PosixSharedMemoryRingBuffer {
public:
    using Buffer = SharedBuffer<EleType>;
    explicit PosixSharedMemoryRingBuffer(const std::string& shmName, size_t size)
        : size_(size),
          shm_(shmName, size * sizeof(EleType) + sizeof(Buffer)),
          producerSem_(shmName + "_producer_sem", size),
          consumerSem_(shmName + "_consumer_sem", 0),
          buffer_(static_cast<Buffer*>(shm_.address())) 
    {
        std::memset(buffer_, 0, size * sizeof(EleType) + sizeof(Buffer));
    }
    
    ~PosixSharedMemoryRingBuffer() = default;

    void put(const EleType& ele) {
        producerSem_.wait();
        buffer_->data[buffer_->tail] = ele; 
        buffer_->tail = (buffer_->tail + 1) % size_;
        consumerSem_.signal();
    }

    template<typename... Args>
    void emplace(Args... args) {
        producerSem_.wait();
        new (buffer_->data + buffer_->tail) EleType(std::forward<Args>(args)...);
        buffer_->tail = (buffer_->tail + 1) % size_;
        consumerSem_.signal();
    }

    EleType get() {
        consumerSem_.wait();
        EleType ele = buffer_->data[buffer_->head];
        buffer_->head = (buffer_->head + 1) % size_;
        producerSem_.signal();
        return ele;
    }

private:
    size_t size_;
    PosixNamedSemaphore<IsOwner> producerSem_;
    PosixNamedSemaphore<IsOwner> consumerSem_;
    PosixSharedMemory<IsOwner> shm_;
    Buffer* buffer_;
};

} // namespace dumbtrader

#endif // DUMBTRADER_IPC_POSIX_SHARED_MEMORY_RING_BUFFER_H_
