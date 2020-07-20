#include "pch.h"
#include "CppUnitTest.h"
#include <cstddef>
#include "../Buddy allocator/Utility.hpp"
#include "../Buddy allocator/Bitset.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(TestUtility)
	{
	public:

		TEST_METHOD(TestStringForNumber)
		{
			std::size_t number = 123456;
			StaticString<64> str = Utility::stringFor(number);
			Assert::IsTrue(str == "123456");
		}
		TEST_METHOD(TestSmallerPowerOf2)
		{
			Assert::AreEqual(Utility::closestSmallerPowerOf2(7), 4UL);
			Assert::AreEqual(Utility::closestSmallerPowerOf2(14), 8UL);
			Assert::AreEqual(Utility::closestSmallerPowerOf2(8), 8UL);
		}
		TEST_METHOD(TestDecToBin)
		{
			Assert::IsTrue(Utility::decToBin(0) == StaticString<64>('0'));
			Assert::IsTrue(Utility::decToBin(1) == StaticString<64>('1'));
			Assert::IsTrue(Utility::decToBin(2) == StaticString<64>("10"));
			Assert::IsTrue(Utility::decToBin(3) == StaticString<64>("11"));
			Assert::IsTrue(Utility::decToBin(4) == StaticString<64>("100"));
			Assert::IsTrue(Utility::decToBin(12) == StaticString<64>("1100"));
			Assert::IsTrue(Utility::decToBin(128) == StaticString<64>("10000000"));
			Assert::IsTrue(Utility::decToBin(130) == StaticString<64>("10000010"));
		}
		TEST_METHOD(TestPtrToHexStr)
		{
			void* ptr1 = reinterpret_cast<void*>(0x00123AA3);
			Assert::IsTrue(Utility::ptrToHexStr(ptr1) == "0x00123AA3");
			void* ptr2 = reinterpret_cast<void*>(0x01000AA0);
			Assert::IsTrue(Utility::ptrToHexStr(ptr2) == "0x01000AA0");

			void* null = reinterpret_cast<void*>(0x00000000);
			Assert::IsTrue(Utility::ptrToHexStr(null) == "0x00000000");
		}
	};

	TEST_CLASS(TestBitset)
	{
	public:

		TEST_METHOD(TestMethod1)
		{
			std::size_t size = 32;
			std::byte* memory = static_cast<std::byte*>(malloc(size));
			Bitset bitset(memory, size);
			Assert::IsFalse(bitset[0]);
			Assert::IsFalse(bitset[7]);
			Assert::IsFalse(bitset[30]);

			bitset.set(12, true);
			Assert::IsTrue(bitset[12]);
			Assert::IsFalse(bitset[11]);
			Assert::IsFalse(bitset[13]);

			bitset.flip(12);
			Assert::IsFalse(bitset[12]);
			Assert::IsFalse(bitset[11]);
			Assert::IsFalse(bitset[13]);

			bitset.set(0, true);
			Assert::IsTrue(bitset[0]);
			Assert::IsFalse(bitset[1]);
			Assert::IsFalse(bitset[2]);

			for (std::size_t i = 1; i <= 32; i++) {
				bitset.set(i, true);
				Assert::IsTrue(bitset[i - 1]);
				Assert::IsTrue(bitset[i]);
				Assert::IsFalse(bitset[i + 1]);
			}

			bitset.set(0, false);
			Assert::IsFalse(bitset[0]);
			Assert::IsTrue(bitset[1]);
			Assert::IsTrue(bitset[2]);

			for (std::size_t i = 1; i < 32; i++) {
				bitset.set(i, false);
				Assert::IsFalse(bitset[i - 1]);
				Assert::IsFalse(bitset[i]);
				Assert::IsTrue(bitset[i + 1]);
			}
			Assert::IsFalse(bitset[33]);

			free(memory);
		}
	};
}
