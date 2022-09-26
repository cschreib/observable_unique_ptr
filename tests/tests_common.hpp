#include "oup/observable_unique_ptr.hpp"

#include <exception>

extern int instances;
extern int instances_derived;
extern int instances_deleter;

extern bool next_test_object_constructor_throws;

struct throw_constructor : std::exception {};

struct test_object {
    enum class state { default_init = 1337, special_init = 42 } state_ = state::default_init;

    test_object() {
        if (next_test_object_constructor_throws) {
            next_test_object_constructor_throws = false;
            throw throw_constructor{};
        }
        ++instances;
    }

    explicit test_object(state s) : state_(s) {
        if (next_test_object_constructor_throws) {
            next_test_object_constructor_throws = false;
            throw throw_constructor{};
        }
        ++instances;
    }

    virtual ~test_object() noexcept {
        --instances;
    }

    test_object(const test_object&) = delete;
    test_object(test_object&&)      = delete;

    test_object& operator=(const test_object&) = delete;
    test_object& operator=(test_object&&) = delete;
};

struct test_object_derived : test_object {
    test_object_derived() {
        ++instances_derived;
    }

    explicit test_object_derived(state s) : test_object(s) {
        ++instances_derived;
    }

    virtual ~test_object_derived() noexcept {
        --instances_derived;
    }
};

struct test_object_observer_from_this_unique :
    public test_object,
    public oup::enable_observer_from_this_unique<test_object_observer_from_this_unique> {
    test_object_observer_from_this_unique() = default;
    test_object_observer_from_this_unique(state s) : test_object(s) {}
};

struct test_object_observer_from_this_sealed :
    public test_object,
    public oup::enable_observer_from_this_sealed<test_object_observer_from_this_sealed> {

    explicit test_object_observer_from_this_sealed(control_block_type& block) :
        oup::enable_observer_from_this_sealed<test_object_observer_from_this_sealed>(block) {}

    explicit test_object_observer_from_this_sealed(control_block_type& block, state s) :
        test_object(s),
        oup::enable_observer_from_this_sealed<test_object_observer_from_this_sealed>(block) {}
};

struct sealed_virtual_policy {
    static constexpr bool is_sealed                            = true;
    static constexpr bool allow_eoft_in_constructor            = false;
    static constexpr bool allow_eoft_multiple_inheritance      = true;
    static constexpr bool eoft_constructor_takes_control_block = false;
    using observer_policy                                      = oup::default_observer_policy;
};

struct unique_non_virtual_policy {
    static constexpr bool is_sealed                            = false;
    static constexpr bool allow_eoft_in_constructor            = true;
    static constexpr bool allow_eoft_multiple_inheritance      = true;
    static constexpr bool eoft_constructor_takes_control_block = true;
    using observer_policy                                      = oup::default_observer_policy;
};

struct unique_maybe_no_block_policy {
    static constexpr bool is_sealed                            = false;
    static constexpr bool allow_eoft_in_constructor            = false;
    static constexpr bool allow_eoft_multiple_inheritance      = true;
    static constexpr bool eoft_constructor_takes_control_block = false;
    using observer_policy                                      = oup::default_observer_policy;
};

struct test_object_observer_from_this_virtual_sealed :
    public test_object,
    public oup::basic_enable_observer_from_this<
        test_object_observer_from_this_virtual_sealed,
        sealed_virtual_policy> {
    test_object_observer_from_this_virtual_sealed() = default;
    test_object_observer_from_this_virtual_sealed(state s) : test_object(s) {}
};

struct test_object_observer_from_this_non_virtual_unique :
    public test_object,
    public oup::basic_enable_observer_from_this<
        test_object_observer_from_this_non_virtual_unique,
        unique_non_virtual_policy> {

    explicit test_object_observer_from_this_non_virtual_unique(control_block_type& block) :
        oup::basic_enable_observer_from_this<
            test_object_observer_from_this_non_virtual_unique,
            unique_non_virtual_policy>(block) {}

    explicit test_object_observer_from_this_non_virtual_unique(control_block_type& block, state s) :
        test_object(s),
        oup::basic_enable_observer_from_this<
            test_object_observer_from_this_non_virtual_unique,
            unique_non_virtual_policy>(block) {}
};

struct test_object_observer_from_this_maybe_no_block_unique :
    public test_object,
    public oup::basic_enable_observer_from_this<
        test_object_observer_from_this_maybe_no_block_unique,
        unique_maybe_no_block_policy> {
    test_object_observer_from_this_maybe_no_block_unique() = default;
    test_object_observer_from_this_maybe_no_block_unique(state s) : test_object(s) {}
};

struct test_object_observer_from_this_derived_unique :
    public test_object_observer_from_this_unique {
    test_object_observer_from_this_derived_unique() = default;
    test_object_observer_from_this_derived_unique(state s) :
        test_object_observer_from_this_unique(s) {}
};

struct test_object_observer_from_this_derived_sealed :
    public test_object_observer_from_this_sealed {

    explicit test_object_observer_from_this_derived_sealed(control_block_type& block) :
        test_object_observer_from_this_sealed(block) {}

    explicit test_object_observer_from_this_derived_sealed(control_block_type& block, state s) :
        test_object_observer_from_this_sealed(block, s) {}
};

struct test_object_observer_from_this_multi_unique :
    public test_object_observer_from_this_unique,
    public oup::enable_observer_from_this_unique<test_object_observer_from_this_multi_unique> {
    test_object_observer_from_this_multi_unique() = default;
    test_object_observer_from_this_multi_unique(state s) :
        test_object_observer_from_this_unique(s) {}
};

struct test_object_observer_from_this_multi_sealed :
    public test_object_observer_from_this_sealed,
    public oup::enable_observer_from_this_sealed<test_object_observer_from_this_multi_sealed> {

    using control_block_type = oup::enable_observer_from_this_sealed<
        test_object_observer_from_this_multi_sealed>::control_block_type;

    explicit test_object_observer_from_this_multi_sealed(control_block_type& block) :
        test_object_observer_from_this_sealed(block),
        oup::enable_observer_from_this_sealed<test_object_observer_from_this_multi_sealed>(block) {}

    explicit test_object_observer_from_this_multi_sealed(control_block_type& block, state s) :
        test_object_observer_from_this_sealed(block, s),
        oup::enable_observer_from_this_sealed<test_object_observer_from_this_multi_sealed>(block) {}
};

struct test_object_observer_from_this_constructor_unique :
    public test_object,
    public oup::enable_observer_from_this_unique<
        test_object_observer_from_this_constructor_unique> {

    oup::observer_ptr<test_object_observer_from_this_constructor_unique> ptr;

    test_object_observer_from_this_constructor_unique() {
        ptr = observer_from_this();
    }

    explicit test_object_observer_from_this_constructor_unique(state s) : test_object(s) {
        ptr = observer_from_this();
    }
};

struct test_object_observer_from_this_constructor_sealed :
    public test_object,
    public oup::enable_observer_from_this_sealed<
        test_object_observer_from_this_constructor_sealed> {

    oup::observer_ptr<test_object_observer_from_this_constructor_sealed> ptr;

    explicit test_object_observer_from_this_constructor_sealed(control_block_type& block) :
        oup::enable_observer_from_this_sealed<test_object_observer_from_this_constructor_sealed>(
            block) {
        ptr = observer_from_this();
    }

    explicit test_object_observer_from_this_constructor_sealed(control_block_type& block, state s) :
        test_object(s),
        oup::enable_observer_from_this_sealed<test_object_observer_from_this_constructor_sealed>(
            block) {
        ptr = observer_from_this();
    }
};

struct test_object_observer_from_this_constructor_bad :
    public test_object,
    public oup::basic_enable_observer_from_this<
        test_object_observer_from_this_constructor_bad,
        sealed_virtual_policy> {

    oup::observer_ptr<test_object_observer_from_this_constructor_bad> ptr;

    explicit test_object_observer_from_this_constructor_bad() {
        ptr = observer_from_this();
    }

    explicit test_object_observer_from_this_constructor_bad(state s) : test_object(s) {
        ptr = observer_from_this();
    }
};

struct test_object_observer_from_this_constructor_multi_unique :
    public test_object_observer_from_this_constructor_unique,
    public oup::enable_observer_from_this_unique<
        test_object_observer_from_this_constructor_multi_unique> {

    oup::observer_ptr<test_object_observer_from_this_constructor_multi_unique> ptr;

    test_object_observer_from_this_constructor_multi_unique() {
        ptr = oup::enable_observer_from_this_unique<
            test_object_observer_from_this_constructor_multi_unique>::observer_from_this();
    }

    test_object_observer_from_this_constructor_multi_unique(state s) :
        test_object_observer_from_this_constructor_unique(s) {
        ptr = oup::enable_observer_from_this_unique<
            test_object_observer_from_this_constructor_multi_unique>::observer_from_this();
    }
};

struct test_object_observer_from_this_constructor_multi_sealed :
    public test_object_observer_from_this_constructor_sealed,
    public oup::enable_observer_from_this_sealed<
        test_object_observer_from_this_constructor_multi_sealed> {

    using control_block_type = oup::enable_observer_from_this_sealed<
        test_object_observer_from_this_constructor_multi_sealed>::control_block_type;

    oup::observer_ptr<test_object_observer_from_this_constructor_multi_sealed> ptr;

    explicit test_object_observer_from_this_constructor_multi_sealed(control_block_type& block) :
        test_object_observer_from_this_constructor_sealed(block),
        oup::enable_observer_from_this_sealed<
            test_object_observer_from_this_constructor_multi_sealed>(block) {
        ptr = oup::enable_observer_from_this_sealed<
            test_object_observer_from_this_constructor_multi_sealed>::observer_from_this();
    }

    explicit test_object_observer_from_this_constructor_multi_sealed(
        control_block_type& block, state s) :
        test_object_observer_from_this_constructor_sealed(block, s),
        oup::enable_observer_from_this_sealed<
            test_object_observer_from_this_constructor_multi_sealed>(block) {
        ptr = oup::enable_observer_from_this_sealed<
            test_object_observer_from_this_constructor_multi_sealed>::observer_from_this();
    }
};

struct test_deleter {
    enum class state {
        default_init   = 1334,
        special_init_1 = 59846,
        special_init_2 = 221,
        empty          = 0
    } state_ = state::default_init;

    test_deleter() noexcept {
        ++instances_deleter;
    }

    explicit test_deleter(state s) noexcept : state_(s) {
        ++instances_deleter;
    }

    test_deleter(const test_deleter& source) noexcept : state_(source.state_) {
        ++instances_deleter;
    }

    test_deleter(test_deleter&& source) noexcept : state_(source.state_) {
        source.state_ = state::empty;
        ++instances_deleter;
    }

    ~test_deleter() noexcept {
        --instances_deleter;
    }

    test_deleter& operator=(const test_deleter&) = default;

    test_deleter& operator=(test_deleter&& source) noexcept {
        state_        = source.state_;
        source.state_ = state::empty;
        return *this;
    }

    void operator()(test_object* ptr) noexcept {
        delete ptr;
    }

    void operator()(std::nullptr_t) noexcept {}
};

template<typename T>
using get_object = typename T::element_type;

template<typename T>
using get_deleter = typename T::deleter_type;

template<typename T>
using get_policy = typename T::policy;

template<typename T>
using get_observer_policy = typename T::observer_policy;

template<typename T>
using observer_ptr = typename T::observer_type;

template<typename T>
constexpr bool is_sealed = get_policy<T>::is_sealed;

template<typename T>
constexpr bool has_stateful_deleter = !std::is_empty_v<get_deleter<T>>;

template<typename T>
constexpr bool has_eoft = oup::has_enable_observer_from_this<get_object<T>, get_policy<T>>;

template<typename T>
constexpr bool    eoft_constructor_takes_control_block =
    has_eoft<T>&& get_policy<T>::eoft_constructor_takes_control_block;

template<typename T>
constexpr bool eoft_allocates = has_eoft<T>&&
               oup::policy_queries<get_policy<T>>::eoft_constructor_allocates();

template<typename T>
constexpr bool must_use_make_observable = is_sealed<T> || eoft_constructor_takes_control_block<T>;

template<typename T>
constexpr bool can_use_make_observable = (is_sealed<T> &&
                                          std::is_same_v<get_deleter<T>, oup::placement_delete>) ||
                                         std::is_same_v<get_deleter<T>, oup::default_delete>;

template<typename T>
constexpr bool has_base = std::is_base_of_v<test_object_derived, get_object<T>>;

template<typename T>
using base_ptr = oup::basic_observable_ptr<
    std::conditional_t<std::is_const_v<get_object<T>>, const test_object, test_object>,
    get_deleter<T>,
    get_policy<T>>;

template<typename T>
using base_observer_ptr = typename base_ptr<T>::observer_type;

template<typename T>
using get_state = std::
    conditional_t<std::is_const_v<get_object<T>>, const test_object::state, test_object::state>;

template<typename T>
using state_observer_ptr = oup::basic_observer_ptr<get_state<T>, get_observer_policy<T>>;

template<typename T>
get_object<T>* make_instance() {
    return new std::remove_cv_t<get_object<T>>;
}

template<typename T>
get_deleter<T> make_deleter_instance_1() {
    return get_deleter<T>(test_deleter::state::special_init_1);
}

template<typename T>
get_deleter<T> make_deleter_instance_2() {
    return get_deleter<T>(test_deleter::state::special_init_2);
}

template<typename T>
T make_pointer_deleter_1() {
    if constexpr (must_use_make_observable<T>) {
        return oup::make_observable<get_object<T>, get_policy<T>>();
    } else {
        if constexpr (has_stateful_deleter<T>) {
            return T(make_instance<T>(), make_deleter_instance_1<T>());
        } else {
            return T(make_instance<T>());
        }
    }
}

template<typename T>
T make_pointer_deleter_2() {
    if constexpr (must_use_make_observable<T>) {
        return oup::make_observable<get_object<T>, get_policy<T>>();
    } else {
        if constexpr (has_stateful_deleter<T>) {
            return T(make_instance<T>(), make_deleter_instance_2<T>());
        } else {
            return T(make_instance<T>());
        }
    }
}

template<typename T>
T make_empty_pointer_deleter_1() {
    if constexpr (has_stateful_deleter<T>) {
        return T(nullptr, make_deleter_instance_1<T>());
    } else {
        return T{};
    }
}

template<typename T>
T make_empty_pointer_deleter_2() {
    if constexpr (has_stateful_deleter<T>) {
        return T(nullptr, make_deleter_instance_2<T>());
    } else {
        return T{};
    }
}

// clang-format off
using owner_types = std::tuple<
    oup::observable_unique_ptr<test_object>,
    oup::observable_sealed_ptr<test_object>,
    oup::observable_unique_ptr<const test_object>,
    oup::observable_sealed_ptr<const test_object>,
    oup::observable_unique_ptr<test_object_derived>,
    oup::observable_sealed_ptr<test_object_derived>,
    oup::observable_unique_ptr<test_object, test_deleter>,
    oup::observable_unique_ptr<test_object_derived, test_deleter>,
    oup::observable_unique_ptr<test_object_observer_from_this_unique>,
    oup::observable_sealed_ptr<test_object_observer_from_this_sealed>,
    oup::basic_observable_ptr<test_object_observer_from_this_non_virtual_unique, oup::default_delete, unique_non_virtual_policy>,
    oup::basic_observable_ptr<test_object_observer_from_this_maybe_no_block_unique, oup::default_delete, unique_maybe_no_block_policy>,
    oup::basic_observable_ptr<test_object_observer_from_this_virtual_sealed, oup::placement_delete, sealed_virtual_policy>,
    oup::observable_unique_ptr<const test_object_observer_from_this_unique>,
    oup::observable_sealed_ptr<const test_object_observer_from_this_sealed>,
    oup::observable_unique_ptr<test_object_observer_from_this_derived_unique>,
    oup::observable_sealed_ptr<test_object_observer_from_this_derived_sealed>,
    oup::observable_unique_ptr<test_object_observer_from_this_multi_unique>,
    oup::observable_sealed_ptr<test_object_observer_from_this_multi_sealed>,
    oup::observable_unique_ptr<test_object_observer_from_this_constructor_unique>,
    oup::observable_sealed_ptr<test_object_observer_from_this_constructor_sealed>,
    oup::observable_unique_ptr<test_object_observer_from_this_constructor_multi_unique>,
    oup::observable_sealed_ptr<test_object_observer_from_this_constructor_multi_sealed>
    >;
// clang-format on
