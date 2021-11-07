#include "speed_benchmark_common.hpp"

template<typename T>
void benchmark<T>::construct_destruct_owner_empty() {
    auto p = owner_type{};
    use_object(p);
}

template<typename T>
void benchmark<T>::construct_destruct_owner() {
    auto p = traits::make_ptr();
    use_object(p);
}

template<typename T>
void benchmark<T>::construct_destruct_owner_factory() {
    auto p = traits::make_ptr_factory();
    use_object(p);
}

template<typename T>
void benchmark<T>::construct_destruct_weak_empty() {
    auto p = weak_type{};
    use_object(p);
}

template<typename T>
void benchmark<T>::construct_destruct_weak() {
    auto wp = traits::make_weak(owner);
    use_object(wp);
}

template<typename T>
void benchmark<T>::construct_destruct_weak_copy() {
    auto wp = weak;
    use_object(wp);
}

template<typename T>
void benchmark<T>::dereference_owner() {
    use_object(*owner);
}

template<typename T>
void benchmark<T>::dereference_weak() {
    traits::deref_weak(weak, [](auto& o) { use_object(o); });
}

template struct benchmark<std::unique_ptr<int>>;
template struct benchmark<std::unique_ptr<float>>;
template struct benchmark<std::unique_ptr<std::string>>;
template struct benchmark<std::unique_ptr<std::array<int,65'536>>>;

template struct benchmark<std::shared_ptr<int>>;
template struct benchmark<std::shared_ptr<float>>;
template struct benchmark<std::shared_ptr<std::string>>;
template struct benchmark<std::shared_ptr<std::array<int,65'536>>>;

template struct benchmark<oup::observable_unique_ptr<int>>;
template struct benchmark<oup::observable_unique_ptr<float>>;
template struct benchmark<oup::observable_unique_ptr<std::string>>;
template struct benchmark<oup::observable_unique_ptr<std::array<int,65'536>>>;

template struct benchmark<oup::observable_sealed_ptr<int>>;
template struct benchmark<oup::observable_sealed_ptr<float>>;
template struct benchmark<oup::observable_sealed_ptr<std::string>>;
template struct benchmark<oup::observable_sealed_ptr<std::array<int,65'536>>>;
