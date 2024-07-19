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
            return dataA > dataB;
        }
        static bool LessThanCondition(T dataA, U dataB) {
            return dataA < dataB;
        }
        static bool EqualToCondition(T dataA, U dataB) {
            return dataA == dataB;
        }
        static bool NotEqualToCondition(T dataA, U dataB) {
            return dataA != dataB;
        }

        void SetDataA(T dataA) { this->dataA = dataA; }
        void SetDataB(U dataB) { this->dataB = dataB; }
    protected:
        GenericTransitionFunc function;
        T dataA;
        U dataB;
    };

    template<class T, class U>
    inline GenericStateTransition<T, U>::GenericStateTransition(const GenericStateTransition& old_transition) : StateTransition(nullptr, nullptr), dataA(old_transition.dataA), dataB(old_transition.dataB)
    {
        this->function = old_transition.function;
    }
}