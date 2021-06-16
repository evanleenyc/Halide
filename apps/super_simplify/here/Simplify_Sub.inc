((a.node_type() == IRNodeType::UNDEFINED) && EVAL_IN_LAMBDA(
((b.node_type() == IRNodeType::Mul) && (
 rewrite(0 - (x*c0), x*fold(0 - c0)))))) ||
((b.node_type() == IRNodeType::Min) && (
 rewrite(x - min(min(x - y, z), w), max(y, x - min(w, z))))) ||
((a.node_type() == IRNodeType::Add) && EVAL_IN_LAMBDA(
 rewrite((min(x, y + z) + w) - y, min(z, x - y) + w) ||
 rewrite((min(x, (y + z) + w) + u) - y, min(x - y, z + w) + u) ||
 rewrite((min(x, (y + z) + w) + u) - z, min(x - z, y + w) + u) ||
((b.node_type() == IRNodeType::Add) && (
 rewrite((x + (y + z)) - (w + z), x + (y - w)) ||
 rewrite((x + ((y*z) + w)) - ((u*y) + v), (y*(z - u)) + ((w + x) - v)) ||
 rewrite((x + ((y*z) + w)) - ((u*z) + v), (z*(y - u)) + ((w + x) - v)) ||
 rewrite((x + (y*z)) - (w + (u*z)), (z*(y - u)) - (w - x)) ||
 rewrite((x + (y*z)) - ((w*y) + u), x + ((y*(z - w)) - u)) ||
 rewrite((x + (y*z)) - ((w*z) + u), x + ((z*(y - w)) - u)) ||
 rewrite(((x + y) + z) - (w + x), (y + z) - w) ||
 rewrite(((x + y) + z) - (w + y), (x + z) - w) ||
 rewrite(((x + (y*z)) + w) - (u + (v*z)), ((z*(y - v)) + w) - (u - x)) ||
 rewrite(((x*y) + z) - (w + ((u*x) + v)), ((y - u)*x) - (w - (z - v))) ||
 rewrite(((x*y) + z) - (w + ((u*y) + v)), ((x - u)*y) - (w - (z - v))) ||
 rewrite(((x*y) + z) - ((w*x) + u), z + ((x*(y - w)) - u)) ||
 rewrite(((x*y) + z) - ((w*y) + u), ((x - w)*y) + (z - u)) ||
 rewrite(((x*y) + z) - ((y*w) + u), ((x - w)*y) + (z - u)))) ||
((b.node_type() == IRNodeType::Min) && (
 rewrite((x + y) - min(z, (w + y) + u), x - min(z - y, w + u)) ||
 rewrite((x + y) - min(z + y, w), x - min(w - y, z)) ||
 rewrite((x + y) - min(min(z, (w + y) + u), v), x - min(min(z, v) - y, u + w)) ||
 rewrite((x + y) - min(min((x + z) + w, u), v), y - min(min(v, u) - x, z + w)))))) ||
((a.node_type() == IRNodeType::Min) && EVAL_IN_LAMBDA(
 rewrite(min((x + y) - z, w) - y, min(x - z, w - y)) ||
 rewrite(min(min(x, (y + z) + w), u) - z, min(min(x, u) - z, y + w)) ||
 rewrite(min(min((x + y) + z, w), u) - y, min(min(w, u) - y, x + z)) ||
 rewrite(min(min(min(x, (y + z) + w), u), v) - z, min(min(x, min(u, v)) - z, y + w)) ||
 rewrite(min(min(min((x + y) + z, w), u), v) - y, min(min(w, min(u, v)) - y, x + z)) ||
((b.node_type() == IRNodeType::Add) && (
 rewrite(min(x, (y + z) + w) - (u + z), min(x - z, y + w) - u))) ||
((b.node_type() == IRNodeType::Mul) && (
 rewrite(min(x, ((y + z)*w) + u) - (z*w), min(x - (w*z), u + (y*w))))))) ||
((a.node_type() == IRNodeType::Max) && EVAL_IN_LAMBDA(
 rewrite(max((x - y) + z, w) - x, max(z - y, w - x)) ||
((b.node_type() == IRNodeType::Add) && (
 rewrite(max(x, y + z) - (w + z), max(y, x - z) - w) ||
 rewrite(max(x, y + (z + w)) - (u + w), max(x - w, z + y) - u) ||
 rewrite(max(x, (y + z) + w) - (u + z), max(x - z, y + w) - u) ||
 rewrite(max(x + y, z) - (w + x), max(z - x, y) - w) ||
 rewrite(max(x + y, z) - (w + y), max(z - y, x) - w)))))
