((b.node_type() == IRNodeType::Add) && (
 rewrite(select(x, y + c0, c1), select(x, y, fold(c1 - c0)) + c0) ||
 rewrite(select(x, (y + z) + w, (u + z) + v), z + select(x, y + w, v + u))))
