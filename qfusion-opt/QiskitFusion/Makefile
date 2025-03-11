QISKIT_AER_ROOT=../qiskit-aer

.PHONY: all gen_build_dir clean

all: static_qiskit.out dynamic_qiskit.out


gen_build_dir:
	mkdir -p ${QISKIT_AER_ROOT}/build/


static_qiskit.out: static_qiskit.hpp gen_build_dir
	@mv ${QISKIT_AER_ROOT}/src/transpile/fusion.hpp ${QISKIT_AER_ROOT}/src/transpile/fusion_backup.hpp # Backup
	@cp $< ${QISKIT_AER_ROOT}/src/transpile/fusion.hpp
	@cd ${QISKIT_AER_ROOT}/build/ && cmake .. -DDISABLE_CONAN=ON -Wno-dev
	@cd ${QISKIT_AER_ROOT}/build/ && cmake --build . --config Release -- -j4
	@mv ${QISKIT_AER_ROOT}/build/Release/qasm_simulator ./$@
	@mv ${QISKIT_AER_ROOT}/src/transpile/fusion_backup.hpp ${QISKIT_AER_ROOT}/src/transpile/fusion.hpp # Recover


dynamic_qiskit.out: dynamic_qiskit.hpp gen_build_dir
	@mv ${QISKIT_AER_ROOT}/src/transpile/fusion.hpp ${QISKIT_AER_ROOT}/src/transpile/fusion_backup.hpp # Backup
	@cp $< ${QISKIT_AER_ROOT}/src/transpile/fusion.hpp
	@cd ${QISKIT_AER_ROOT}/build/ && cmake .. -DDISABLE_CONAN=ON -Wno-dev
	@cd ${QISKIT_AER_ROOT}/build/ && cmake --build . --config Release -- -j4
	@mv ${QISKIT_AER_ROOT}/build/Release/qasm_simulator ./$@
	@mv ${QISKIT_AER_ROOT}/src/transpile/fusion_backup.hpp ${QISKIT_AER_ROOT}/src/transpile/fusion.hpp # Recover

clean:
	@if [ -e "${QISKIT_AER_ROOT}/src/transpile/fusion_backup.hpp" ]; then \
		mv ${QISKIT_AER_ROOT}/src/transpile/fusion_backup.hpp ${QISKIT_AER_ROOT}/src/transpile/fusion.hpp; \
	fi