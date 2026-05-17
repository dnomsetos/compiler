fn val(x: &i32) -> i32 {
    *x
}

fn add(a: &i32, b: &i32) -> i32 {
    *a + *b
}

fn mul(a: &i32, b: &i32) -> i32 {
    *a * *b
}

fn main() {
    let a = 1;
    let b = 2;
    let c = 3;
    let d = 4;
    let e = 5;
    let f = 6;

    let mut r = &a;
    print_i32(*r);

    r = &b;
    print_i32(*r);

    r = { let t = &c; t };
    print_i32(*r);

    r = if 1 == 1 { &d } else { &e };
    print_i32(*r);

    r = { let t = &f; t };
    print_i32(*r);

    {
        let mut s = &a;
        print_i32(*s);

        s = &e;
        print_i32(*s);

        s = &c;
        print_i32(add(r, s));
    }

    {
        let mut t = &b;
        print_i32(val(t));

        t = &d;
        print_i32(mul(t, &b));

        t = { let k = &e; k };
        print_i32(add(t, &f));
    }

    let mut x = &a;
    let mut y = &b;

    x = &c;
    y = &d;
    print_i32(add(x, y));

    x = { let p = &e; p };
    y = { let q = &f; q };
    print_i32(mul(x, y));

    let mut i = 0;
    let mut p = &a;

    'outer: loop {
        p = if i == 0 {
            &b
        } else {
            if i == 1 {
                &c
            } else {
                &d
            }
        };

        print_i32(*p);

        i = i + 1;
        if i == 3 {
            break 'outer;
        }

        continue 'outer;
    }

    let mut j = 0;
    let mut q = &f;

    'inner: loop {
        q = if j == 0 {
            &a
        } else {
            if j == 1 {
                &d
            } else {
                &e
            }
        };

        print_i32(*q);

        j = j + 1;
        if j == 3 {
            break 'inner;
        }

        continue 'inner;
    }

    {
        let mut z = &a;
        print_i32(*z);

        z = &b;
        print_i32(*z);

        z = &c;
        print_i32(*z);

        z = &d;
        print_i32(*z);

        z = &e;
        print_i32(*z);

        z = &f;
        print_i32(*z);
    }
}
