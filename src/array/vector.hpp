namespace oak {
    template <typename T>
    class Vector {
    private:
        int m_capacity { 2 };
        int m_size {};
        T* m_data { nullptr };

        void allocate(int size) {
            // TODO: Add std::nothrow ?
            m_data = new T[size];
            if (!m_data) {
                std::cerr << "Failed to allocate m_data\n";
            }
        }
    public:
        // Default constructor
        Vector() = default;

        // Constructor with size
        Vector(size_t n) : m_size(m_size) {
            // Allocate
        }

        // Destructor
        ~Vector() {
            delete m_data;
            m_data = nullptr;
        }
    };
}