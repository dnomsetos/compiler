fn add_one(x: &i32) -> i32 {
    *x + 1
}

fn add_two(x: &i32) -> i32 {
    let y = add_one(x);
    y + 1
}

fn sum_three(a: &i32, b: &i32, c: &i32) -> i32 {
    *a + *b + *c
}

fn main() {
    let a = 10;
    let b = 20;
    let c = 30;

    let ra = &a;
    let rb = &b;
    let rc = &c;

    let s1 = sum_three(ra, rb, rc);
    print_i32(s1);

    let s2 = add_two(ra);
    print_i32(s2);

    {
        let inner = &a;
        let v = *inner + 5;
        print_i32(v);
    }

    let again = add_one(&b);
    print_bool(again == 21);

    let chain1 = add_two(&c);
    let chain2 = add_one(&chain1);
    print_bool(chain2 == 33);
}
