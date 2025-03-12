QISKIT_AER_ROOT=../qiskit-aer

.PHONY: all gen_build_dir finally clean

all: static_qiskit.out dynamic_qiskit.out finally


gen_build_dir:
	mkdir -p ${QISKIT_AER_ROOT}/build/


static_qiskit.out: static_qiskit.hpp gen_build_dir
	@cp $< ${QISKIT_AER_ROOT}/src/transpile/fusion.hpp # patch the fusion.hpp
	@cd ${QISKIT_AER_ROOT}/build/ && cmake .. -DDISABLE_CONAN=ON && \
		cmake --build . --config Release -- -j8 2>| compile.log
	@mv ${QISKIT_AER_ROOT}/build/Release/qasm_simulator ./$@


dynamic_qiskit.out: dynamic_qiskit.hpp gen_build_dir
	@cp $< ${QISKIT_AER_ROOT}/src/transpile/fusion.hpp  # patch the fusion.hpp
	@cd ${QISKIT_AER_ROOT}/build/ && cmake .. -DDISABLE_CONAN=ON && \
		cmake --build . --config Release -- -j8 2>| compile.log
	@mv ${QISKIT_AER_ROOT}/build/Release/qasm_simulator ./$@


finally:
	@cp ./fusion.hpp ${QISKIT_AER_ROOT}/src/transpile/fusion.hpp # Recover


clean:
	@if [ -e "${QISKIT_AER_ROOT}/src/transpile/fusion_backup.hpp" ]; then \
		mv ${QISKIT_AER_ROOT}/src/transpile/fusion_backup.hpp ${QISKIT_AER_ROOT}/src/transpile/fusion.hpp; \
	fi