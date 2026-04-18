namespace SweetLine.Demo {

    import Std.Core.*;
    import Std.Arrays.*;
    import Std.Math.*;
    import Std.Measurement.*;
    import Std.Diagnostics.*;

    /// A small helper type to exercise type declarations.
    newtype Angle = Double;

    newtype Pair = (Int, Int);

    function Clamp(value : Int, minValue : Int, maxValue : Int) : Int {
        if (value < minValue) {
            return minValue;
        } elif (value > maxValue) {
            return maxValue;
        } else {
            return value;
        }
    }

    function IsEven(value : Int) : Bool {
        return value % 2 == 0;
    }

    function Summarize(values : Int[]) : Int {
        mutable total = 0;
        for value in values {
            set total += value;
        }
        return total;
    }

    operation FlipIfZero(q : Qubit, value : Int) : Unit is Adj + Ctl {
        if (value == 0) {
            X(q);
        } else {
            Z(q);
        }
    }

    operation PrepareState(angle : Angle, q : Qubit) : Unit {
        H(q);
        Rz(angle!, q);
    }

    operation MeasureAndReset(q : Qubit) : Result {
        let result = M(q);
        if (result == One) {
            Reset(q);
        }
        return result;
    }

    operation RepeatUntilOne(q : Qubit) : Unit {
        repeat {
            H(q);
            let current = M(q);
        } until M(q) == One fixup {
            Reset(q);
        }
    }

    operation UseAndBorrowDemo() : Unit {
        use q = Qubit();
        borrowing (r = Qubit()) {
            within {
                H(q);
            } apply {
                CNOT(q, r);
            }
        }
        ResetAll([q, r]);
    }

    operation NestedControlDemo(q : Qubit) : Unit is Adj + Ctl {
        within {
            H(q);
        } apply {
            T(q);
            Adjoint S(q);
        }
    }

    function PairFirst(pair : Pair) : Int {
        let (first, second) = pair!;
        return first;
    }

    operation IndexedDemo(values : Int[]) : Unit {
        for idx in 0 .. Length(values) - 1 {
            Message($"idx={idx}, value={values[idx]}");
        }
    }

    @EntryPoint()
    operation Main() : Unit {
        use q = Qubit();
        use r = Qubit();

        let angle = Angle(0.125);
        let pair = Pair(2, 5);
        let clamped = Clamp(12, 0, 10);
        let even = IsEven(clamped);
        let values = [1, 2, 3, 4, 5];
        let total = Summarize(values);

        Message($"pair={PairFirst(pair)}");
        Message($"even={even}");
        Message($"total={total}");

        PrepareState(angle, q);
        FlipIfZero(r, total);
        _ = MeasureAndReset(q);

        repeat {
            H(r);
        } until M(r) == One fixup {
            Reset(r);
        }

        for item in values {
            Message($"item={item}");
        }

        NestedControlDemo(r);
        ResetAll([q, r]);
    }
}

