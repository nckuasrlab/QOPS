GATE_OPS = {
    "H": 0,
    "S": 1,
    "T": 2,
    "X": 3,
    "Y": 4,
    "Z": 5,
    "Phase": 6,
    "U1": 7,
    "CX": 8,
    "CY": 9,
    "CZ": 10,
    "CPhase": 11,
    "CU1": 12,
    "SWAP": 13,
    "TOFFOLI": 14,
    "M": 20,
    "OPS_MEASURE_MULTI": 21,
    "OPS_COPY": 22,
    "U2": 31,
    "U3": 32,
}
INV_GATE_OPS = {v: k for k, v in GATE_OPS.items()}

import graphviz


def get_involved_qubits(data):
    ret = {}
    for di in data:
        if len(di) == 1:
            break
        ret[di[1]] = 1
        if di[0] in [1, 3]:  # control, swap
            ret[di[2]] = 1
    return ret


def v_connect(v, nodes, has_gate):
    with v.subgraph() as t:
        t.attr(rank="same")
        # t.attr("edge", style="dashed")
        for idx in range(len(nodes) - 1):
            style, penwidth = (
                ("solid", "4")
                if (len(has_gate) > 1 and idx <= max(has_gate) and idx > min(has_gate))
                else ("dashed", "1")
            )
            if (
                (idx - 1 in has_gate and idx in has_gate)
                or (idx - 1 in has_gate and nodes[idx + 1].startswith("td"))
                or (idx in has_gate and nodes[idx].startswith("tu"))
            ):
                t.edge(
                    f"{nodes[idx]}:s",
                    f"{nodes[idx+1]}:n",
                    style=style,
                    penwidth=penwidth,
                )
            elif idx - 1 in has_gate or nodes[idx].startswith("tu"):
                t.edge(
                    f"{nodes[idx]}:s", f"{nodes[idx+1]}", style=style, penwidth=penwidth
                )
            elif idx in has_gate or nodes[idx + 1].startswith("td"):
                t.edge(
                    f"{nodes[idx]}", f"{nodes[idx+1]}:n", style=style, penwidth=penwidth
                )
            else:
                t.edge(
                    f"{nodes[idx]}", f"{nodes[idx+1]}", style=style, penwidth=penwidth
                )


def main():
    data = []
    with open("../simplifiedStateVector/src/default.profdata") as fin:
        data = [[int(di) for di in line.split()] for line in fin]

    MAX_QUB = max(get_involved_qubits(data).keys()) + 1  # 1 for q_0
    MAX_SEQ = len(data)

    g = graphviz.Digraph(name="CIRC", strict=True)
    g.attr(margin="0")
    g.attr(
        "graph",
        rankdir="LR",
        overlap="false",
        splines="ortho",
        nodesep="0.1",
        ranksep="0.2",
        ordering="out",
        newrank="true",  # https://stackoverflow.com/a/18410951
    )
    g.attr("edge", arrowhead="none", headclip="false", tailclip="false")
    g.attr(
        "node",
        shape="square",
        width="0.4",
        height="0.4",
        label="",
        fontsize="8",
        fixedsize="true",
    )

    with g.subgraph() as q:
        q.attr(rank="same")
        q.attr("node", shape="plaintext")
        q.node("tu0", label="", height="0.0")
        q.node("td0", label="Time (us)", width="0.6")
        for qi in range(MAX_QUB):
            q.node(f"map_0_{qi}", label=f"q_{qi}")

    with g.subgraph() as c:
        c.attr("node", shape="plaintext", fontsize="8")
        c.attr("edge", style="invis", constraint="true")
        for ti in range(MAX_SEQ):
            c.edge(f"tu{ti}", f"tu{ti+1}")
            c.edge(f"td{ti}", f"td{ti+1}")
            if ti > 0:
                c.node(
                    f"td{ti}",
                    label=f"{(data[ti][-1]-data[ti-1][-1])/1e3:.1f}",
                )
        c.edge("tu0", "map_0_0")
        c.edge(f"map_0_{MAX_QUB-1}", "td0")
        for qi in range(MAX_QUB - 1):
            c.edge(f"map_0_{qi}", f"map_0_{qi+1}")

    with g.subgraph() as v:
        v.attr("edge", style="solid", weight="6")
        v.attr("node", shape="square")
        gate_map = [list(range(MAX_QUB))]
        for idx, di in enumerate(data):
            ti = idx + 1
            has_gate = []

            if di[0] == 0:  # single_gate
                gname = INV_GATE_OPS[di[2]]
                v.node(f"map_{ti}_{di[1]}", label=gname)
                has_gate.append(di[1])
            elif di[0] == 1:  # control_gate
                gname = INV_GATE_OPS[di[3]]
                # use a cluster to wrap the dot of control bit to unify the
                # height
                with v.subgraph(name=f"cluster_{ti}_{di[1]}") as p:
                    p.attr(margin="8.9", color="white")
                    p.attr("node", shape="point", width="0.15")
                    p.node(f"map_{ti}_{di[1]}")
                v.node(f"map_{ti}_{di[2]}", label=gname[1:])
                has_gate.append(di[1])
                has_gate.append(di[2])
            elif di[0] == 2:  # unitary4x4
                pass
            elif di[0] == 3:  # SWAP
                gname = INV_GATE_OPS[13]
                v.node(f"map_{ti}_{di[1]}", label=gname)
                v.node(f"map_{ti}_{di[2]}", label=gname)
                has_gate.append(di[1])
                has_gate.append(di[2])
            elif di[0] == 4:  # unitary8x8
                pass

            gate_map.append(has_gate)
            for qi in range(MAX_QUB):
                # draw dummy node and be overwrited if has gate
                if qi not in has_gate:
                    v.node(f"map_{ti}_{qi}", label="", color="white")

        for ti in range(0, MAX_SEQ):
            if ti > 0:
                v_connect(
                    v,
                    [
                        f"tu{ti}",
                        *[f"map_{ti}_{qi}" for qi in range(MAX_QUB)],
                        f"td{ti}",
                    ],
                    gate_map[ti],
                )
            for qi in range(MAX_QUB):
                if qi in gate_map[ti] and qi in gate_map[ti + 1]:
                    v.edge(f"map_{ti}_{qi}:e", f"map_{ti+1}_{qi}:w")
                elif qi in gate_map[ti]:
                    v.edge(f"map_{ti}_{qi}:e", f"map_{ti+1}_{qi}")
                elif qi in gate_map[ti + 1]:
                    v.edge(f"map_{ti}_{qi}", f"map_{ti+1}_{qi}:w")
                else:
                    v.edge(f"map_{ti}_{qi}", f"map_{ti+1}_{qi}")

    g.render("g.gv", view=False)


if __name__ == "__main__":
    main()
