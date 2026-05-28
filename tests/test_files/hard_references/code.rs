fn add(x: &mut i32, n: i32) {
    *x = *x + n;
}

fn mul(x: &mut i32, n: i32) {
    *x = *x * n;
}

fn read(x: &i32) -> i32 {
    *x
}

fn combine(a: &i32, b: &i32) -> i32 {
    *a + *b
}

fn main() {
    let mut a = 1;
    let mut b = 2;
    let mut c = 3;
    let mut d = 4;

    {
        let ra = &a;
        let rb = &b;

        let s = combine(ra, rb);
        print_i32(s);
    }

    {
        let x = &mut a;
        add(x, 10);
        mul(x, 2);

        print_i32(*x);
    }

    {
        let y = &mut b;
        add(y, 5);
        mul(y, 3);

        print_i32(*y);
    }

    print_i32(a);
    print_i32(b);

    'outer: loop {
        {
            let rc = &c;
            let rd = &d;

            let s1 = read(rc);
            let s2 = read(rd);

            print_i32(s1);
            print_i32(s2);

            if s1 == 3 {
                if s2 == 4 {
                    break 'outer;
                }
            }

            continue 'outer;
        }
    }

    'modify: loop {
        {
            let mc = &mut c;

            add(mc, 7);

            print_i32(*mc);

            if *mc == 10 {
                break 'modify;
            }

            continue 'modify;
        }
    }

    print_i32(c);

    'nested: loop {
        {
            let md = &mut d;

            mul(md, 5);

            print_i32(*md);

            if *md == 20 {
                {
                    let r1 = &a;
                    let r2 = &b;
                    let r3 = &c;
                    let r4 = &d;

                    let x1 = combine(r1, r2);
                    let x2 = combine(r3, r4);

                    print_i32(x1);
                    print_i32(x2);
                }

                break 'nested;
            }

            continue 'nested;
        }
    }

    {
        let ra = &a;
        let rb = &b;
        let rc = &c;
        let rd = &d;

        let total1 = combine(ra, rb);
        let total2 = combine(rc, rd);

        print_i32(total1);
        print_i32(total2);
    }

    'control: loop {
        {
            let x = &a;

            print_i32(*x);

            if *x == 22 {
                break 'control;
            }

            continue 'control;
        }
    }

    {
        let ma = &mut a;
        let mb = &mut b;

        add(ma, 1);
        add(mb, 2);

        print_i32(*ma);
        print_i32(*mb);
    }

    {
        let final_a = &a;
        let final_b = &b;
        let final_c = &c;
        let final_d = &d;

        print_i32(*final_a);
        print_i32(*final_b);
        print_i32(*final_c);
        print_i32(*final_d);
    }
}
