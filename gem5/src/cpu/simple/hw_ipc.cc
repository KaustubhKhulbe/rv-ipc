#include "cpu/simple/hw_ipc.hh"

namespace gem5
{
    namespace hw_ipc {
        
        int fifo_num;
        int fifo_depth;
        int fifo_size;
        std::unordered_map<int, fifo_control> fifo_list;
        std::mutex fifo_mutex;

        

        HwIpc::HwIpc(const HwIpcParams &params) : 
            SimObject(params)
        {
            fifo_num = params.fifo_num;
            fifo_depth = params.fifo_depth;

            std::cout << "HwIpc constructor" << std::endl;
        }

        HwIpc::~HwIpc()
        {
            std::cout << "HwIpc destructor" << std::endl;
        }

        void HwIpc::fcreate(SimpleExecContext *xc, StaticInstPtr inst)
        {
            fifo_mutex.lock();
            
            RegVal id = xc->getRegOperand(inst.get(), 0); // rs1
            RegVal key = xc->getRegOperand(inst.get(), 1); // rs2

            // Check a fifo with same id already exists
            if (fifo_list.find(id) != fifo_list.end()) {
                fifo_mutex.unlock();

                xc->setRegOperand(inst.get(), 0, RegVal(-1)); // Return -1
                return;
            }

            // Allocate a new fifo
            fifo_list[id] = fifo_control();
            fifo_list[id].fifo = std::queue<std::vector<uint64_t>>();
            fifo_list[id].busy = false;
            fifo_list[id].key = key;
            fifo_list[id].idle_cycles = 0;
            

            // printf("Fcreate: Id: %ld, Key: %ld\n",id, key);
            xc->setRegOperand(inst.get(), 0, RegVal(0)); // Return 0
            fifo_mutex.unlock();
        }

        void HwIpc::fconn(SimpleExecContext *xc, StaticInstPtr inst)
        {
            fifo_mutex.lock();

            RegVal id = xc->getRegOperand(inst.get(), 0); // rs1
            RegVal key = xc->getRegOperand(inst.get(), 1); // rs2
            
            if (fifo_list.find(id) == fifo_list.end()) { // Fifo does not exist
                fifo_mutex.unlock();
                xc->setRegOperand(inst.get(), 0, RegVal(-1));
                return;
            }

            if (fifo_list[id].key != key) { // Key is not correct
                fifo_mutex.unlock();
                xc->setRegOperand(inst.get(), 0, RegVal(-1));
                return;
            }

            if (fifo_list[id].busy) { // Used already
                fifo_mutex.unlock();
                xc->setRegOperand(inst.get(), 0, RegVal(-1));
                return;
            }

            fifo_list[id].busy = true;
            fifo_list[id].idle_cycles = 0;

            // printf("Fconn: Id: %ld, Key: %ld\n",id, key);
            xc->setRegOperand(inst.get(), 0, RegVal(0)); // Return 0
            fifo_mutex.unlock();
        }

        void HwIpc::fsend(SimpleExecContext *xc, StaticInstPtr inst)
        {
            fifo_mutex.lock();


            RegVal id = xc->getRegOperand(inst.get(), 0); // rs1
            RegVal key = xc->getRegOperand(inst.get(), 1); // rs2
            
            if (fifo_list.find(id) == fifo_list.end()) { // Fifo does not exist
                fifo_mutex.unlock();
                xc->setRegOperand(inst.get(), 0, RegVal(-1));
                return;
            }

            if (fifo_list[id].key != key || !fifo_list[id].busy) { // Key is not correct or fifo is not busy
                fifo_mutex.unlock();
                xc->setRegOperand(inst.get(), 0, RegVal(-2));
                return;
            }

            if (fifo_list[id].fifo.size() >= fifo_depth) { // Fifo is full
                fifo_mutex.unlock();
                xc->setRegOperand(inst.get(), 0, RegVal(-3));
                return;
            }

            
            RegClass int_reg_class = RegClass(IntRegClass, IntRegClassName, RiscvISA::int_reg::NumRegs, debug::IntRegs);
            RegId r_idx;
            std::vector<uint64_t> data(4);
        
            for (int i = 0; i < 4; i++) {
                r_idx = RegId(int_reg_class, 28 + i);
                data[i] = xc->thread->getReg(r_idx);
            }

            fifo_list[id].fifo.push(data);

            xc->setRegOperand(inst.get(), 0, RegVal(0)); // Return 0
            fifo_mutex.unlock();
        }

        void HwIpc::frecv(SimpleExecContext *xc, StaticInstPtr inst)
        {
            fifo_mutex.lock();

            RegVal id = xc->getRegOperand(inst.get(), 0); // rs1
            RegVal key = xc->getRegOperand(inst.get(), 1); // rs2
            
            if (fifo_list.find(id) == fifo_list.end()) { // Fifo does not exist
                fifo_mutex.unlock();
                xc->setRegOperand(inst.get(), 0, RegVal(-1));
                return;
            }

            if (fifo_list[id].key != key || !fifo_list[id].busy) { // Key is not correct or fifo is not busy
                fifo_mutex.unlock();
                xc->setRegOperand(inst.get(), 0, RegVal(-2));
                return;
            }

            if (fifo_list[id].fifo.empty()) { // Fifo is empty
                fifo_mutex.unlock();
                xc->setRegOperand(inst.get(), 0, RegVal(-3));
                return;
            }
            
            std::vector<uint64_t> data = fifo_list[id].fifo.front();
            fifo_list[id].fifo.pop();

            RegClass int_reg_class = RegClass(IntRegClass, IntRegClassName, RiscvISA::int_reg::NumRegs, debug::IntRegs);
            RegId r_idx;

            for (int i = 0; i < 4; i++) {
                r_idx = RegId(int_reg_class, 28 + i);
                xc->thread->setReg(r_idx, data[i]);
            }

            xc->setRegOperand(inst.get(), 0, RegVal(0)); // Return 0
            fifo_mutex.unlock();
        }

        void HwIpc::fclose(SimpleExecContext *xc, StaticInstPtr inst)
        {
            std::cout << "HwIpc fclose" << std::endl;
        }
    } // namespace hw_ipc
} // namespace gem5
