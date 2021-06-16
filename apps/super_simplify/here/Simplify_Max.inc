((b.node_type() == IRNodeType::Add) && (
 rewrite(max(x, max(max(x, y), z) + c0), max(max(x, y), z) + fold(max(c0, -1)), max(min(c0, 0), -1) == 0))) ||
((a.node_type() == IRNodeType::Add) && EVAL_IN_LAMBDA(
 rewrite(max(max(max(x, y), z) + c0, y), max(max(x, y), z) + fold(max(c0, -1)), max(min(c0, 0), -1) == 0) ||
((b.node_type() == IRNodeType::Add) && (
 rewrite(max(x + (y + z), (w + z) + u), z + max(y + x, w + u)) ||
 rewrite(max(x + (y + z), (y + w) + u), y + max(x + z, u + w)) ||
 rewrite(max(x + (y - z), y + w), max(w, x - z) + y) ||
 rewrite(max((x + y) + z, (y + w) + u), y + max(x + z, u + w)) ||
 rewrite(max((x - y) + z, w + (u - y)), max(x + z, w + u) - y))))) ||
((a.node_type() == IRNodeType::Sub) && EVAL_IN_LAMBDA(
((b.node_type() == IRNodeType::Sub) && (
 rewrite(max(x - y, z - (y + w)), max(x, z - w) - y))))) ||
((a.node_type() == IRNodeType::Min) && EVAL_IN_LAMBDA(
((b.node_type() == IRNodeType::Min) && (
 rewrite(max(min(min(x, y), z), min(w, y)), min(y, max(min(x, z), w))))))) ||
((a.node_type() == IRNodeType::Max) && EVAL_IN_LAMBDA(
((b.node_type() == IRNodeType::Add) && (
 rewrite(max(max(x, y + z), (w + z) + u), max(x, z + max(y, w + u))) ||
 rewrite(max(max(x, y + z), (y + w) + u), max(x, y + max(z, u + w))) ||
 rewrite(max(max(x + y, z), (w + x) + u), max(z, x + max(y, w + u))) ||
 rewrite(max(max(x - y, z), ((w - y) + u) + v), max(z, max(x, v + (w + u)) - y)) ||
 rewrite(max(max(max(x, y + z), w), u + z), max(max(z + max(y, u), w), x)) ||
 rewrite(max(max(max(x + y, z), w), u + y), max(max(max(x, u) + y, w), z)) ||
 rewrite(max(max(max(max(x, y + z), w), u), v + z), max(max(max(z + max(y, v), w), u), x)))) ||
((b.node_type() == IRNodeType::Min) && (
 rewrite(max(max(min(x, y), z), min(w, y)), max(z, min(max(x, w), y))) ||
 rewrite(max(max(min(x, y), z), min(y, w)), max(z, min(max(x, w), y))) ||
 rewrite(max(max(max(min(x, y), z), w), min(u, y)), max(max(min(max(x, u), y), w), z)))))) ||
((a.node_type() == IRNodeType::Select) && EVAL_IN_LAMBDA(
 rewrite(max(select(x, y, z), z), select(x, max(y, z), z))))
