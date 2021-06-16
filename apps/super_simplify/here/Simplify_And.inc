((a.node_type() == IRNodeType::And) && EVAL_IN_LAMBDA(
((b.node_type() == IRNodeType::LE) && (
 rewrite(((x <= y) && z) && (x <= w), z && (x <= min(y, w)))))))
