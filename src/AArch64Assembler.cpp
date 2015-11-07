#include <assert.h>
#include <stdexcept>
#include "AArch64Assembler.h"

CAArch64Assembler::CAArch64Assembler()
{

}

CAArch64Assembler::~CAArch64Assembler()
{
	
}

void CAArch64Assembler::SetStream(Framework::CStream* stream)
{
	m_stream = stream;
}

CAArch64Assembler::LABEL CAArch64Assembler::CreateLabel()
{
	return m_nextLabelId++;
}

void CAArch64Assembler::ClearLabels()
{
	m_labels.clear();
}

void CAArch64Assembler::MarkLabel(LABEL label)
{
	m_labels[label] = static_cast<size_t>(m_stream->Tell());
}

void CAArch64Assembler::CreateLabelReference(LABEL label, CONDITION condition)
{
	LABELREF reference;
	reference.offset = static_cast<size_t>(m_stream->Tell());
	reference.condition = condition;
	m_labelReferences.insert(std::make_pair(label, reference));
}

void CAArch64Assembler::ResolveLabelReferences()
{
	for(const auto& labelReferencePair : m_labelReferences)
	{
		auto label(m_labels.find(labelReferencePair.first));
		if(label == m_labels.end())
		{
			throw std::runtime_error("Invalid label.");
		}
		const auto& labelReference = labelReferencePair.second;
		size_t labelPos = label->second;
		int offset = static_cast<int>(labelPos - labelReference.offset) / 4;

		m_stream->Seek(labelReference.offset, Framework::STREAM_SEEK_SET);
		if(labelReference.condition == CONDITION_AL)
		{
			uint32 opcode = 0x14000000;
			opcode |= (offset & 0x3FFFFFF);
			WriteWord(opcode);
		}
		else
		{
			uint32 opcode = 0x54000000;
			opcode |= (offset & 0x7FFFF) << 5;
			opcode |= labelReference.condition;
			WriteWord(opcode);
		}
	}
	m_stream->Seek(0, Framework::STREAM_SEEK_END);
	m_labelReferences.clear();
}

void CAArch64Assembler::Add(REGISTER32 rd, REGISTER32 rn, REGISTER32 rm)
{
	uint32 opcode = 0x0B000000;
	opcode |= (rd <<  0);
	opcode |= (rn <<  5);
	opcode |= (rm << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Add(REGISTER64 rd, REGISTER64 rn, REGISTER64 rm)
{
	uint32 opcode = 0x8B000000;
	opcode |= (rd <<  0);
	opcode |= (rn <<  5);
	opcode |= (rm << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Add(REGISTER32 rd, REGISTER32 rn, uint16 imm, ADDSUB_IMM_SHIFT_TYPE shift)
{
	WriteAddSubOpImm(0x11000000, shift, imm, rn, rd);
}

void CAArch64Assembler::Add(REGISTER64 rd, REGISTER64 rn, uint16 imm, ADDSUB_IMM_SHIFT_TYPE shift)
{
	WriteAddSubOpImm(0x91000000, shift, imm, rn, rd);
}

void CAArch64Assembler::And(REGISTER32 rd, REGISTER32 rn, REGISTER32 rm)
{
	uint32 opcode = 0x0A000000;
	opcode |= (rd <<  0);
	opcode |= (rn <<  5);
	opcode |= (rm << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::And(REGISTER32 rd, REGISTER32 rn, uint8 immr, uint8 imms)
{
	WriteLogicalOpImm(0x12000000, immr, imms, rn, rd);
}

void CAArch64Assembler::Asr(REGISTER32 rd, REGISTER32 rn, uint8 sa)
{
	uint32 imms = 0x1F;
	uint32 immr = sa & 0x1F;
	WriteLogicalOpImm(0x13000000, immr, imms, rn, rd);
}

void CAArch64Assembler::Asr(REGISTER64 rd, REGISTER64 rn, uint8 sa)
{
	uint32 imms = 0x3F;
	uint32 immr = sa & 0x3F;
	WriteLogicalOpImm(0x93400000, immr, imms, rn, rd);
}

void CAArch64Assembler::Asrv(REGISTER32 rd, REGISTER32 rn, REGISTER32 rm)
{
	WriteDataProcOpReg2(0x1AC02800, rm, rn, rd);
}

void CAArch64Assembler::Asrv(REGISTER64 rd, REGISTER64 rn, REGISTER64 rm)
{
	WriteDataProcOpReg2(0x9AC02800, rm, rn, rd);
}

void CAArch64Assembler::B(LABEL label)
{
	CreateLabelReference(label, CONDITION_AL);
	WriteWord(0);
}

void CAArch64Assembler::Bl(uint32 offset)
{
	assert((offset & 0x3) == 0);
	offset /= 4;
	assert(offset < 0x40000000);
	uint32 opcode = 0x94000000;
	opcode |= offset;
	WriteWord(opcode);
}

void CAArch64Assembler::BCc(CONDITION condition, LABEL label)
{
	CreateLabelReference(label, condition);
	WriteWord(0);
}

void CAArch64Assembler::Blr(REGISTER64 rn)
{
	uint32 opcode = 0xD63F0000;
	opcode |= (rn << 5);
	WriteWord(opcode);
}

void CAArch64Assembler::Cmn(REGISTER32 rn, uint16 imm, ADDSUB_IMM_SHIFT_TYPE shift)
{
	WriteAddSubOpImm(0x31000000, shift, imm, rn, wZR);
}

void CAArch64Assembler::Cmn(REGISTER64 rn, uint16 imm, ADDSUB_IMM_SHIFT_TYPE shift)
{
	WriteAddSubOpImm(0xB1000000, shift, imm, rn, wZR);
}

void CAArch64Assembler::Cmp(REGISTER32 rn, REGISTER32 rm)
{
	uint32 opcode = 0x6B000000;
	opcode |= (wZR << 0);
	opcode |= (rn  << 5);
	opcode |= (rm  << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Cmp(REGISTER64 rn, REGISTER64 rm)
{
	uint32 opcode = 0xEB000000;
	opcode |= (wZR << 0);
	opcode |= (rn  << 5);
	opcode |= (rm  << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Cmp(REGISTER32 rn, uint16 imm, ADDSUB_IMM_SHIFT_TYPE shift)
{
	WriteAddSubOpImm(0x71000000, shift, imm, rn, wZR);
}

void CAArch64Assembler::Cmp(REGISTER64 rn, uint16 imm, ADDSUB_IMM_SHIFT_TYPE shift)
{
	WriteAddSubOpImm(0xF1000000, shift, imm, rn, wZR);
}

void CAArch64Assembler::Cset(REGISTER32 rd, CONDITION condition)
{
	uint32 opcode = 0x1A800400;
	opcode |= (rd  << 0);
	opcode |= (wZR << 5);
	opcode |= ((condition ^ 1) << 12);	//Inverting lsb inverts condition
	opcode |= (wZR << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Eor(REGISTER32 rd, REGISTER32 rn, REGISTER32 rm)
{
	uint32 opcode = 0x4A000000;
	opcode |= (rd <<  0);
	opcode |= (rn <<  5);
	opcode |= (rm << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Ldp_PostIdx(REGISTER64 rt, REGISTER64 rt2, REGISTER64 rn, int32 offset)
{
	assert((offset & 0x07) == 0);
	int32 scaledOffset = offset / 8;
	assert(scaledOffset >= -64 && scaledOffset <= 63);
	uint32 opcode = 0xA8C00000;
	opcode |= (rt  <<  0);
	opcode |= (rn  <<  5);
	opcode |= (rt2 << 10);
	opcode |= ((scaledOffset & 0x7F) << 15);
	WriteWord(opcode);
}

void CAArch64Assembler::Ldr(REGISTER32 rt, REGISTER64 rn, uint32 offset)
{
	assert((offset & 0x03) == 0);
	uint32 scaledOffset = offset / 4;
	assert(scaledOffset < 0x1000);
	WriteLoadStoreOpImm(0xB9400000, scaledOffset, rn, rt);
}

void CAArch64Assembler::Ldr(REGISTER64 rt, REGISTER64 rn, uint32 offset)
{
	assert((offset & 0x07) == 0);
	uint32 scaledOffset = offset / 8;
	assert(scaledOffset < 0x1000);
	WriteLoadStoreOpImm(0xF9400000, scaledOffset, rn, rt);
}

void CAArch64Assembler::Lsl(REGISTER32 rd, REGISTER32 rn, uint8 sa)
{
	uint32 imms = 0x1F - (sa & 0x1F);
	uint32 immr = -sa & 0x1F;
	WriteLogicalOpImm(0x53000000, immr, imms, rn, rd);
}

void CAArch64Assembler::Lsl(REGISTER64 rd, REGISTER64 rn, uint8 sa)
{
	uint32 imms = 0x3F - (sa & 0x3F);
	uint32 immr = -sa & 0x3F;
	WriteLogicalOpImm(0xD3400000, immr, imms, rn, rd);
}

void CAArch64Assembler::Lslv(REGISTER32 rd, REGISTER32 rn, REGISTER32 rm)
{
	WriteDataProcOpReg2(0x1AC02000, rm, rn, rd);
}

void CAArch64Assembler::Lslv(REGISTER64 rd, REGISTER64 rn, REGISTER64 rm)
{
	WriteDataProcOpReg2(0x9AC02000, rm, rn, rd);
}

void CAArch64Assembler::Lsr(REGISTER32 rd, REGISTER32 rn, uint8 sa)
{
	uint32 imms = 0x1F;
	uint32 immr = sa & 0x1F;
	WriteLogicalOpImm(0x53000000, immr, imms, rn, rd);
}

void CAArch64Assembler::Lsr(REGISTER64 rd, REGISTER64 rn, uint8 sa)
{
	uint32 imms = 0x3F;
	uint32 immr = sa & 0x3F;
	WriteLogicalOpImm(0xD3400000, immr, imms, rn, rd);
}

void CAArch64Assembler::Lsrv(REGISTER32 rd, REGISTER32 rn, REGISTER32 rm)
{
	WriteDataProcOpReg2(0x1AC02400, rm, rn, rd);
}

void CAArch64Assembler::Lsrv(REGISTER64 rd, REGISTER64 rn, REGISTER64 rm)
{
	WriteDataProcOpReg2(0x9AC02400, rm, rn, rd);
}

void CAArch64Assembler::Mov(REGISTER32 rd, REGISTER32 rm)
{
	uint32 opcode = 0x2A0003E0;
	opcode |= (rd <<  0);
	opcode |= (rm << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Mov(REGISTER64 rd, REGISTER64 rm)
{
	uint32 opcode = 0xAA0003E0;
	opcode |= (rd <<  0);
	opcode |= (rm << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Mov_Sp(REGISTER64 rd, REGISTER64 rn)
{
	uint32 opcode = 0x91000000;
	opcode |= (rd << 0);
	opcode |= (rn << 5);
	WriteWord(opcode);
}

void CAArch64Assembler::Movn(REGISTER32 rd, uint16 imm, uint8 pos)
{
	assert(pos < 2);
	WriteMoveWideOpImm(0x12800000, pos, imm, rd);
}

void CAArch64Assembler::Movk(REGISTER32 rd, uint16 imm, uint8 pos)
{
	assert(pos < 2);
	WriteMoveWideOpImm(0x72800000, pos, imm, rd);
}

void CAArch64Assembler::Movk(REGISTER64 rd, uint16 imm, uint8 pos)
{
	assert(pos < 4);
	WriteMoveWideOpImm(0xF2800000, pos, imm, rd);
}

void CAArch64Assembler::Movz(REGISTER32 rd, uint16 imm, uint8 pos)
{
	assert(pos < 2);
	WriteMoveWideOpImm(0x52800000, pos, imm, rd);
}

void CAArch64Assembler::Movz(REGISTER64 rd, uint16 imm, uint8 pos)
{
	assert(pos < 4);
	WriteMoveWideOpImm(0xD2800000, pos, imm, rd);
}

void CAArch64Assembler::Msub(REGISTER32 rd, REGISTER32 rn, REGISTER32 rm, REGISTER32 ra)
{
	uint32 opcode = 0x1B008000;
	opcode |= (rd << 0);
	opcode |= (rn << 5);
	opcode |= (ra << 10);
	opcode |= (rm << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Mvn(REGISTER32 rd, REGISTER32 rm)
{
	uint32 opcode = 0x2A200000;
	opcode |= (rd  << 0);
	opcode |= (wZR << 5);
	opcode |= (rm  << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Orr(REGISTER32 rd, REGISTER32 rn, REGISTER32 rm)
{
	uint32 opcode = 0x2A000000;
	opcode |= (rd <<  0);
	opcode |= (rn <<  5);
	opcode |= (rm << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Ret(REGISTER64 rn)
{
	uint32 opcode = 0xD65F0000;
	opcode |= (rn << 5);
	WriteWord(opcode);
}

void CAArch64Assembler::Sdiv(REGISTER32 rd, REGISTER32 rn, REGISTER32 rm)
{
	uint32 opcode = 0x1AC00C00;
	opcode |= (rd <<  0);
	opcode |= (rn <<  5);
	opcode |= (rm << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Smull(REGISTER64 rd, REGISTER32 rn, REGISTER32 rm)
{
	uint32 opcode = 0x9B200000;
	opcode |= (rd  <<  0);
	opcode |= (rn  <<  5);
	opcode |= (wZR << 10);
	opcode |= (rm  << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Str(REGISTER32 rt, REGISTER64 rn, uint32 offset)
{
	assert((offset & 0x03) == 0);
	uint32 scaledOffset = offset / 4;
	assert(scaledOffset < 0x1000);
	WriteLoadStoreOpImm(0xB9000000, scaledOffset, rn, rt);
}

void CAArch64Assembler::Str(REGISTER64 rt, REGISTER64 rn, uint32 offset)
{
	assert((offset & 0x07) == 0);
	uint32 scaledOffset = offset / 8;
	assert(scaledOffset < 0x1000);
	WriteLoadStoreOpImm(0xF9000000, scaledOffset, rn, rt);
}

void CAArch64Assembler::Stp_PreIdx(REGISTER64 rt, REGISTER64 rt2, REGISTER64 rn, int32 offset)
{
	assert((offset & 0x07) == 0);
	int32 scaledOffset = offset / 8;
	assert(scaledOffset >= -64 && scaledOffset <= 63);
	uint32 opcode = 0xA9800000;
	opcode |= (rt  <<  0);
	opcode |= (rn  <<  5);
	opcode |= (rt2 << 10);
	opcode |= ((scaledOffset & 0x7F) << 15);
	WriteWord(opcode);
}

void CAArch64Assembler::Sub(REGISTER32 rd, REGISTER32 rn, REGISTER32 rm)
{
	uint32 opcode = 0x4B000000;
	opcode |= (rd << 0);
	opcode |= (rn << 5);
	opcode |= (rm << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Sub(REGISTER32 rd, REGISTER32 rn, uint16 imm, ADDSUB_IMM_SHIFT_TYPE shift)
{
	WriteAddSubOpImm(0x51000000, shift, imm, rn, rd);
}

void CAArch64Assembler::Sub(REGISTER64 rd, REGISTER64 rn, uint16 imm, ADDSUB_IMM_SHIFT_TYPE shift)
{
	WriteAddSubOpImm(0xD1000000, shift, imm, rn, rd);
}

void CAArch64Assembler::Udiv(REGISTER32 rd, REGISTER32 rn, REGISTER32 rm)
{
	uint32 opcode = 0x1AC00800;
	opcode |= (rd <<  0);
	opcode |= (rn <<  5);
	opcode |= (rm << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::Umull(REGISTER64 rd, REGISTER32 rn, REGISTER32 rm)
{
	uint32 opcode = 0x9BA00000;
	opcode |= (rd  <<  0);
	opcode |= (rn  <<  5);
	opcode |= (wZR << 10);
	opcode |= (rm  << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::WriteAddSubOpImm(uint32 opcode, uint32 shift, uint32 imm, uint32 rn, uint32 rd)
{
	assert(imm < 0x1000);
	opcode |= (rd << 0);
	opcode |= (rn << 5);
	opcode |= ((imm & 0xFFF) << 10);
	opcode |= (shift << 22);
	WriteWord(opcode);
}

void CAArch64Assembler::WriteDataProcOpReg2(uint32 opcode, uint32 rm, uint32 rn, uint32 rd)
{
	opcode |= (rd <<  0);
	opcode |= (rn <<  5);
	opcode |= (rm << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::WriteLogicalOpImm(uint32 opcode, uint32 immr, uint32 imms, uint32 rn, uint32 rd)
{
	opcode |= (rd << 0);
	opcode |= (rn << 5);
	opcode |= (imms << 10);
	opcode |= (immr << 16);
	WriteWord(opcode);
}

void CAArch64Assembler::WriteLoadStoreOpImm(uint32 opcode, uint32 imm, uint32 rn, uint32 rt)
{
	opcode |= (rt << 0);
	opcode |= (rn << 5);
	opcode |= (imm << 10);
	WriteWord(opcode);
}

void CAArch64Assembler::WriteMoveWideOpImm(uint32 opcode, uint32 hw, uint32 imm, uint32 rd)
{
	opcode |= (rd << 0);
	opcode |= (imm << 5);
	opcode |= (hw << 21);
	WriteWord(opcode);
}

void CAArch64Assembler::WriteWord(uint32 value)
{
	m_stream->Write32(value);
}
