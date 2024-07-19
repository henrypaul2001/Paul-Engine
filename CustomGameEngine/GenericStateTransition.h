#pragma once
#include "StateTransition.h"
namespace Engine {
    template <class T, class U>
    class GenericStateTransition : public StateTransition
    {
    public:
        typedef bool (*GenericTransitionFunc)(T, U);

        GenericStateTransition(const GenericStateTransition& old_transition);
        GenericStateTransition(GenericTransitionFunc function, T data, U otherData, State* source, State* destination) : StateTransition(source, destination), dataA(data), dataB(otherData) {
            this->function = function;
        }
        ~GenericStateTransition() {}

        StateTransition* Copy() override { return new GenericStateTransition<T, U>(*this); }

        virtual bool Condition() const override {
            if (function) {
                return function(dataA, dataB);
            }
            return false;
        }

        // Generic condition functions
        static bool GreaterThanCondition(T dataA, U dataB) {
            return Compare(dataA, dataB, std::greater<>{});
        }
        static bool LessThanCondition(T dataA, U dataB) {
            return Compare(dataA, dataB, std::less<>{});
        }
        static bool EqualToCondition(T dataA, U dataB) {
            return Compare(dataA, dataB, std::equal_to<>{});
        }
        static bool NotEqualToCondition(T dataA, U dataB) {
            return Compare(dataA, dataB, std::not_equal_to<>{});
        }

        void SetDataA(T newDataA) { this->dataA = newDataA; }
        void SetDataB(U newDataB) { this->dataB = newDataB; }
    protected:
        GenericTransitionFunc function;
        T dataA;
        U dataB;

    private:
        // Helper function to compare pointers or values
        template <typename V, typename W, typename Comp>
        static bool Compare(V a, W b, Comp comp) {
            if constexpr (std::is_pointer_v<V> && std::is_pointer_v<W>) {
                return comp(*a, *b);
            }
            else if constexpr (std::is_pointer_v<V>) {
                return comp(*a, b);
            }
            else if constexpr (std::is_pointer_v<W>) {
                return comp(a, *b);
            }
            else {
                return comp(a, b);
            }
        }
    };

    template<class T, class U>
    inline GenericStateTransition<T, U>::GenericStateTransition(const GenericStateTransition& old_transition) : StateTransition(nullptr, nullptr), dataA(old_transition.dataA), dataB(old_transition.dataB)
    {
        this->function = old_transition.function;
    }
}