((a.node_type() == IRNodeType::Add) && EVAL_IN_LAMBDA(
((b.node_type() == IRNodeType::Add) && (
 rewrite(min(x + y, (y - z) + w), y + min(w - z, x)) ||
 rewrite(min(x + (y + z), w + z), min(w, y + x) + z) ||
 rewrite(min(x + (y + z), (w + z) + u), z + min(y + x, w + u)) ||
 rewrite(min(x + (y - z), (w + y) + u), y + min(x - z, u + w)) ||
 rewrite(min(x + (y - z), ((w + y) + u) + v), y + min(x - z, (v + w) + u)) ||
 rewrite(min(x + ((y + z) - w), (u + z) + w), z + min(y + (x - w), w + u)) ||
 rewrite(min((x + (y - z)) + w, (u + y) + v), y + min(x + (w - z), u + v)) ||
 rewrite(min((x - y) + z, (w + x) + u), x + min(z - y, u + w)))))) ||
((a.node_type() == IRNodeType::Sub) && EVAL_IN_LAMBDA(
((b.node_type() == IRNodeType::Add) && (
 rewrite(min(x - y, ((z - y) + w) + u), min(x, (u + z) + w) - y) ||
 rewrite(min(x - (y + z), (w - z) + u), min(x - y, w + u) - z) ||
 rewrite(min((x + y) - z, w + y), y + min(x - z, w)) ||
 rewrite(min((x + y) - z, (w + (u + y)) + v), y + min(x - z, w + (u + v))) ||
 rewrite(min((x + y) - z, (x + y) + w), y + min(x - z, x + w)) ||
 rewrite(min((x + y) - z, ((y - w) + u) + v), y + min(x - z, (v - w) + u)) ||
 rewrite(min((x + y) - z, ((x + y) - w) + u), y + min(x - z, (u + x) - w)))) ||
((b.node_type() == IRNodeType::Sub) && (
 rewrite(min(x - (y + z), w - y), min(w, x - z) - y) ||
 rewrite(min((x - y) - z, x - w), x - max(y + z, w)))))) ||
((a.node_type() == IRNodeType::Min) && EVAL_IN_LAMBDA(
 rewrite(min(min(x, y + c0), y), min(x, y), 0 <= c0) ||
 rewrite(min(min(x + c0, y), x), min(x, y), 0 <= c0) ||
 rewrite(min(min(min(min(min(min(x, y), z), w), u), v), x), min(min(min(min(min(y, z), w), u), v), x)) ||
 rewrite(min(min(min(min(min(min(x, y), z), w), u), v), y), min(min(min(min(min(x, z), w), u), v), y)) ||
((b.node_type() == IRNodeType::Add) && (
 rewrite(min(min(x, (y - z) + w), (u - z) + v), min(x, min(w + y, v + u) - z)) ||
 rewrite(min(min(x, y - z), (w - z) + u), min(x, min(y, u + w) - z)) ||
 rewrite(min(min(x, min(y + z, w)), u + z), min(min(x, min(y, u) + z), w)) ||
 rewrite(min(min(x, min(min(y, z + w), u)), v + w), min(min(x, min(w + min(z, v), u)), y)) ||
 rewrite(min(min(x + y, z), (w + y) + u), min(z, y + min(x, w + u))) ||
 rewrite(min(min(x + y, z), (x - w) + u), min(z, x + min(y, u - w))) ||
 rewrite(min(min((x + y) + z, w), y + u), min(w, min(x + z, u) + y)) ||
 rewrite(min(min((x - y) + z, w), (u - y) + v), min(w, min(x + z, v + u) - y)) ||
 rewrite(min(min(min(x, y), z), y + c0), min(min(x, z), y), 0 <= c0) ||
 rewrite(min(min(min(x, y + z), w), u + z), min(min(z + min(y, u), w), x)) ||
 rewrite(min(min(min(x + y, z), w), u + y), min(min(min(x, u) + y, w), z)) ||
 rewrite(min(min(min(min(x, y), z), w), y + c0), min(min(min(x, z), w), y), 0 <= c0) ||
 rewrite(min(min(min(min(x, y + z), w), u), v + z), min(min(min(z + min(y, v), w), u), x)) ||
 rewrite(min(min(min(min(x + y, z), w), u), v + y), min(min(min(min(x, v) + y, w), u), z)))) ||
((b.node_type() == IRNodeType::Sub) && (
 rewrite(min(min(x, (y - z) + w), u - z), min(x, min(u, y + w) - z)) ||
 rewrite(min(min(x, y - z), w - z), min(x, min(y, w) - z)) ||
 rewrite(min(min(x, y - z), y - w), min(x, y - max(z, w))) ||
 rewrite(min(min(x, min(y - z, w)), y - u), min(x, min(y - max(z, u), w))) ||
 rewrite(min(min(x - y, z), w - y), min(z, min(x, w) - y)) ||
 rewrite(min(min(x - y, z), x - w), min(z, x - max(y, w))) ||
 rewrite(min(min(min(x, y - z), w), y - u), min(min(y - max(u, z), w), x)) ||
 rewrite(min(min(min(x, min(y - z, w)), u), y - v), min(w, min(x, min(y - max(z, v), u)))) ||
 rewrite(min(min(min(x - y, z), w), x - u), min(min(x - max(u, y), w), z)) ||
 rewrite(min(min(min(min(x - y, z), w), u), x - v), min(min(x - max(y, v), u), min(w, z))))) ||
((b.node_type() == IRNodeType::Mul) && (
 rewrite(min(min(x, (y*z) + w), y*z), min(x, min(w, 0) + (y*z))) ||
 rewrite(min(min((x*y) + z, w), x*y), min(w, min(z, 0) + (x*y))))))) ||
((a.node_type() == IRNodeType::Max) && EVAL_IN_LAMBDA(
((b.node_type() == IRNodeType::Add) && (
 rewrite(min(max(min(x, y), z), max(x, z) + c0), max(min(x, y), z), 0 <= c0))) ||
((b.node_type() == IRNodeType::Min) && (
 rewrite(min(max(x, y), min(max(z, y), w)), min(max(y, min(x, z)), w))))))
