{
    __builtin_unreachable();
}
void f(qreg q)
{
    void __internal_call_function_f(qreg);
    __internal_call_function_f(q);
}
class f : public qcor::QuantumKernel<class f, qreg>
{
    friend class qcor::QuantumKernel<class f, qreg>;
    friend class qcor::KernelSignature<qreg>;

protected:
    void operator()(qreg q)
    {
        if (!parent_kernel)
        {
            parent_kernel = qcor::__internal__::create_composite(kernel_name);
        }
        quantum::set_current_program(parent_kernel);
        if (runtime_env == QrtType::FTQC)
        {
            quantum::set_current_buffer(q.results());
        }
        init_kernel_signature_args(parent_kernel, q);
        for (int i = 0; i < 10; i++)
        {
            quantum::h(q[i]);
            quantum::mz(q[i]);
        }
    }

public:
    inline static const std::string kernel_name = "f";
    f(qreg q) : QuantumKernel<f, qreg>(q) {}
    f(std::shared_ptr<qcor::CompositeInstruction> _parent, qreg q) : QuantumKernel<f, qreg>(_parent, q) {}
    virtual ~f()
    {
        if (disable_destructor)
        {
            return;
        }
        auto [q] = args_tuple;
        operator()(q);
        if (runtime_env == QrtType::FTQC)
        {
            if (is_callable)
            {
                quantum::persistBitstring(q.results());
                for (size_t shotCount = 1; shotCount < quantum::get_shots(); ++shotCount)
                {
                    operator()(q);
                    quantum::persistBitstring(q.results());
                }
                quantum::submit(nullptr);
            }
            return;
        }
        if (optimize_only)
        {
            xacc::internal_compiler::execute_pass_manager();
            return;
        }
        if (is_callable)
        {
            xacc::internal_compiler::execute_pass_manager();
            quantum::submit(q.results());
        }
    }
};
void f(std::shared_ptr<qcor::CompositeInstruction> parent, qreg q)
{
    class f __ker__temp__(parent, q);
}
void __internal_call_function_f(qreg q)
{
    class f __ker__temp__(q);
}