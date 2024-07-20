#pragma once
#include "StateTransition.h"
#include <functional>
namespace Engine {
    template <class T, class U, class RA, class RB>
    class GenericStateTransition : public StateTransition
    {
    public:
        typedef bool (*GenericTransitionFunc)(RA, RB);

        typedef RA(*GenericDataRetrieverFuncA)(GenericStateTransition*);
        typedef RB(*GenericDataRetrieverFuncB)(GenericStateTransition*);

        GenericStateTransition(const GenericStateTransition& old_transition);
        GenericStateTransition(GenericTransitionFunc function, T baseData, U otherBaseData, State* source, State* destination, GenericDataRetrieverFuncA dataRetrieverA, GenericDataRetrieverFuncB dataRetrieverB) : StateTransition(source, destination), dataA(baseData), dataB(otherBaseData) {
            this->function = function;
            this->retrieverA = dataRetrieverA;
            this->retrieverB = dataRetrieverB;
        }
        ~GenericStateTransition() {}

        StateTransition* Copy() override { return new GenericStateTransition<T, U, RA, RB>(*this); }

        virtual bool Condition() override {
            if (function) {
                return function(retrieverA(this), retrieverB(this));
            }
            return false;
        }

        // Generic condition functions
        static bool GreaterThanCondition(RA dataA, RB dataB) {
            return Compare(dataA, dataB, std::greater<>{});
        }
        static bool LessThanCondition(RA dataA, RB dataB) {
            return Compare(dataA, dataB, std::less<>{});
        }
        static bool EqualToCondition(RA dataA, RB dataB) {
            return Compare(dataA, dataB, std::equal_to<>{});
        }
        static bool NotEqualToCondition(RA dataA, RB dataB) {
            return Compare(dataA, dataB, std::not_equal_to<>{});
        }

        // Generic data retrieval functions
        static RA PassthroughA(GenericStateTransition* owner) { return owner->GetBaseDataA(); }
        static RB PassthroughB(GenericStateTransition* owner) { return owner->GetBaseDataB(); }

        void SetBaseDataA(T newDataA) { this->dataA = newDataA; }
        void SetBaseDataB(U newDataB) { this->dataB = newDataB; }

        const T& GetBaseDataA() const { return dataA; }
        const U& GetBaseDataB() const { return dataB; }

    protected:
        GenericTransitionFunc function;

        T dataA;
        U dataB;

        GenericDataRetrieverFuncA retrieverA;
        GenericDataRetrieverFuncB retrieverB;

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

    template <class T, class U, class RA, class RB>
    inline GenericStateTransition<T, U, RA, RB>::GenericStateTransition(const GenericStateTransition& old_transition) : StateTransition(nullptr, nullptr), dataA(old_transition.dataA), dataB(old_transition.dataB)
    {
        this->function = old_transition.function;
        this->retrieverA = old_transition.retrieverA;
        this->retrieverB = old_transition.retrieverB;
    }
}