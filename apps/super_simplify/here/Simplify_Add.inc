((a.node_type() == IRNodeType::Add) && EVAL_IN_LAMBDA(
 rewrite(((select(x, c0, c1) + y) + z) + c2, (select(x, fold(c0 + c2), fold(c1 + c2)) + z) + y) ||
 rewrite(((x*c0) + ((y*c1) + z)) + w, (z + ((y - x)*fold(0 - c0))) + w, (c0 + c1) == 0) ||
 rewrite((select(x, c0, c1) + y) + c2, y + select(x, fold(c0 + c2), fold(c1 + c2))))) ||
((a.node_type() == IRNodeType::Sub) && EVAL_IN_LAMBDA(
 rewrite((x - min(y, z + w)) + w, x - min(z, y - w)) ||
 rewrite((x - min(y + z, w)) + z, x - min(w - z, y)) ||
 rewrite((x - min(y - z, w)) + y, x + max(z, y - w)) ||
 rewrite((x - min(min(y, (z + x) + w), u)) + v, v - min(min(y, u) - x, z + w)) ||
 rewrite((x - min(min((y + x) + z, w), u)) + v, v - min(min(w, u) - x, y + z)) ||
((b.node_type() == IRNodeType::Add) && (
 rewrite((x - y) + (z + (w + y)), x + (z + w)) ||
 rewrite((x - y) + (z + (w - x)), z + (w - y)) ||
 rewrite((x - y) + ((y + z) + w), x + (z + w)) ||
 rewrite((x - y) + ((z + y) + w), x + (z + w)) ||
 rewrite((x - y) + ((z - x) + w), (w - y) + z) ||
 rewrite((x - (y + z)) + (w + y), x - (z - w)) ||
 rewrite((x - (y + z)) + (w + z), x - (y - w)) ||
 rewrite((x - (y + z)) + ((w + z) + x), (x - y) + (w + x)) ||
 rewrite((x - (y + (z*w))) + (u + (v*w)), (w*(v - z)) + (u + (x - y))) ||
 rewrite((x - (y + (z*w))) + ((u*w) + v), (w*(u - z)) + ((x - y) + v)) ||
 rewrite((x - ((y*z) + w)) + (u + (v*z)), (z*(v - y)) + (u + (x - w))) ||
 rewrite((x - ((y*z) + w)) + ((u*z) + v), (z*(u - y)) + ((x - w) + v)) ||
 rewrite((x - ((y*z) + w)) + ((z*u) + v), (z*(u - y)) + ((x - w) + v)))))) ||
((a.node_type() == IRNodeType::Mul) && EVAL_IN_LAMBDA(
((b.node_type() == IRNodeType::Add) && (
 rewrite((x*c0) + (y + ((z*c1) + w)), w + (y + ((z - x)*c1)), (c0 + c1) == 0) ||
 rewrite((x*y) + (z + (w + ((u*x) + v))), v + (z + (w + ((u + y)*x)))) ||
 rewrite((x*y) + (z + (w + ((u*y) + v))), v + (z + (w + ((u + x)*y)))) ||
 rewrite((x*y) + (z + (w + ((x*u) + v))), v + (z + (w + ((y + u)*x)))) ||
 rewrite((x*y) + (z + ((w*x) + u)), u + (z + ((w + y)*x))) ||
 rewrite((x*y) + (z + ((w*y) + u)), u + (z + ((w + x)*y))) ||
 rewrite((x*y) + (z + (w*y)), z + (y*(x + w))) ||
 rewrite((x*y) + ((z + (w*y)) + u), z + ((y*(w + x)) + u)) ||
 rewrite((x*y) + ((x*z) + w), w + (x*(y + z))) ||
 rewrite((x*y) + ((z*x) + w), (x*(y + z)) + w) ||
 rewrite((x*y) + ((z*y) + w), (y*(x + z)) + w))) ||
((b.node_type() == IRNodeType::Sub) && (
 rewrite((x*y) + (z - (w + ((u*x) + v))), z - (v + (w - ((y - u)*x)))) ||
 rewrite((x*y) + (z - ((w*x) + u)), ((x*(y - w)) - u) + z) ||
 rewrite((x*y) + ((z*x) - w), (x*(y + z)) - w) ||
 rewrite((x*y) + ((z*y) - w), (y*(x + z)) - w))))) ||
((a.node_type() == IRNodeType::Select) && EVAL_IN_LAMBDA(
 rewrite(select(x, y, z + c0) + c1, select(x, y + c1, z), (c0 + c1) == 0)))
