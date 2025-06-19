/*
 * \brief  Timestamp counter probe with histogram data
 * \author Johannes Schlatow
 * \date   2025-04-02
 */

/*
 * Copyright (C) 2025 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _TSC_HISTOGRAM_H_
#define _TSC_HISTOGRAM_H_

#include <base/output.h>
#include <base/buffered_output.h>
#include <base/mutex.h>
#include <trace/timestamp.h>

namespace Genode {
	template <unsigned,unsigned,unsigned,typename> class Histogram;
	template <typename> class Tsc_hist_probe;
}


/*
 * Arranges timestamp values in a histogram.
 *
 * The number of bins is defined by 'BINS'.
 * Added timestamp values must be in the interval [RANGE_START,RANGE_END[.
 */
template <unsigned BINS, unsigned RANGE_START, unsigned RANGE_END, typename CONV>
class Genode::Histogram : Noncopyable
{
	private:

		static constexpr unsigned const _bin_size =
			(RANGE_END-RANGE_START+BINS-1)/BINS;

		using Type   = CONV::Type;
		using Format = CONV::Format;

		CONV     &_conv;
		unsigned  _bins[BINS];
		Type      _sum = 0;
		Type      _max = 0;
		Type      _min = ~(Type)0;
		unsigned  _maxed_out = 0;
		unsigned  _num_samples = 0;

		template <typename FN>
		void _with_bin(Type value, FN && fn)
		{
			if (value < RANGE_START || value >= RANGE_END) {
				/* only report maxed out error once */
				if (!_maxed_out)
					error("Value ", value, " is out of range");
				if (value >= RANGE_END)
					_maxed_out++;
				return;
			}

			unsigned b = (unsigned)((value - RANGE_START)/_bin_size);
			fn(_bins[b]);
		}

		template <typename FN>
		void _for_each_bin(FN && fn)
		{
			for (unsigned b = 0; b < BINS; b++)
				fn(_bins[b], b);
		}

		template <typename FN>
		void _for_each_bin(FN && fn) const
		{
			for (unsigned b = 0; b < BINS; b++)
				fn(_bins[b], b);
		}

		struct Hist_bar
		{
			unsigned const bar_size;
			unsigned const quantum;
			unsigned char  symbol;

			void print(Output &out) const
			{
				for (unsigned i=(bar_size+quantum-1)/quantum; i; i--)
					Genode::print(out, Char(symbol));
			}

			Hist_bar(unsigned size, unsigned quantum, char symbol)
			: bar_size(size), quantum(quantum), symbol(symbol)
			{ }
		};

	public:

		Type average() const
		{
			if (!_num_samples)
				return 0;

			return _sum / _num_samples;
		}

		Type max_value() const
		{
			return _max;
		}

		/*
		 * returns the approximate p-th percentile
		 */
		Type percentile(unsigned p) const
		{
			if (!_num_samples)
				return 0;

			if (p > 100) p = 100;

			unsigned const index = (_num_samples * p + 99) / 100;
			unsigned bin_start = RANGE_START;
			unsigned samples = 0;
			unsigned result  = 0;
			_for_each_bin([&] (unsigned const &bin, unsigned) {
				if (index >= samples && index < samples + bin)
					result = bin_start + _bin_size/2;

				bin_start += _bin_size;
				samples   += bin;
			});

			return result;
		}

		void add(Trace::Timestamp timestamp)
		{
			Type sample = _conv.convert(timestamp);

			_sum = _sum + sample;
			_max = max(_max, sample);
			_min = min(_min, sample);
			_num_samples++;

			/* place into bin */
			_with_bin(sample, [&] (unsigned &bin) {
				bin++;
			});
		}

		Histogram &operator += (Histogram const &rhs)
		{
			_sum       += rhs._sum;
			_maxed_out += rhs._maxed_out;
			_max = max(_max, rhs._max);
			_min = min(_min, rhs._min);
			_num_samples += rhs._num_samples;

			_for_each_bin([&] (unsigned &bin, unsigned index) {
				bin += rhs._bins[index];
			});

			return *this;
		}

		template <typename OUTPUT_FN>
		void output_stats() const
		{
			/* print min, max, average, etc. */
			OUTPUT_FN("#samples: ", _num_samples);
			OUTPUT_FN("minimum:  ", Format{_min});
			OUTPUT_FN("maximum:  ", Format{_max});
			OUTPUT_FN("average:  ", Format{average()});
		}

		template <typename OUTPUT_FN>
		void output_visual_stats() const
		{
			Type const median    = percentile(50);
			Type const quartile1 = percentile(25);
			Type const quartile3 = percentile(75);
			Type const p90       = percentile(90);

			/*
			 * visual representation of bins as follows;
			 * |  >   ---------+----  * < |
			 * > = bin contains minimum
			 * < = bin contains maximum (if not maxed_out)
			 * - = bins in inter-quartile range
			 * + = median
			 * * = bin contains 90th percentile
			 */

			int before_min    = -1;
			int before_q1     = -1;
			int before_median = -1;
			int before_q3     = -1;
			int before_p90    = -1;
			int before_max    = -1;
			_for_each_bin([&] (unsigned const &, unsigned index) {
				Type const bin_start = RANGE_START + (index * _bin_size);

				if (_min      > bin_start) before_min++;
				if (quartile1 > bin_start) before_q1++;
				if (median    > bin_start) before_median++;
				if (quartile3 > bin_start) before_q3++;
				if (p90       > bin_start) before_p90++;
				if (_max      > bin_start) before_max++;
			});

			bool     const print_min    = before_median > before_min;
			bool     const print_p90    = before_p90 > before_q3;
			bool     const print_max    = (before_max > before_p90)
			                           && (!_maxed_out)
			                           && ((int)BINS - before_max);
			bool     const print_maxout = _maxed_out;

			if (print_min && before_q1 == before_min)
				before_q1++;

			unsigned const bar1 = before_min;
			unsigned const bar2 = max(0,before_q1-before_min-(int)print_min);
			unsigned const bar3 = max(0,before_median-before_q1);
			unsigned const bar4 = max(0,before_q3-before_median);
			unsigned const bar5 = max(0,before_p90-before_q3-1);
			unsigned const bar6 = max(0,before_max-before_p90-1);
			unsigned const bar7 = max(0,(int)BINS-before_max-2);

			OUTPUT_FN("|",
			          Hist_bar(bar1, 1, ' '),
			          print_min ? ">" : "",
			          Hist_bar(bar2, 1, ' '),
			          Hist_bar(bar3, 1, '-'),
			          "+",
			          Hist_bar(bar4, 1, '-'),
			          Hist_bar(bar5, 1, ' '),
			          print_p90 ? "*" : "",
			          Hist_bar(bar6, 1, ' '),
			          print_max ? "<" : "",
			          Hist_bar(bar7, 1, ' '),
			          "|",
			          print_maxout ? " <" : "", " ", _num_samples);
		}

		template <typename OUTPUT_FN>
		void output_histogram() const
		{
			if (!_num_samples) {
				OUTPUT_FN("no samples");
				return;
			}

			/* conduct precalculations */
			unsigned largest_bin = 0;
			_for_each_bin([&] (unsigned const &bin, unsigned) {
				largest_bin = max(largest_bin, bin);
			});

			/* print histogram */
			unsigned const quantum   = (largest_bin + 59) / 60;
			unsigned const median    = (_num_samples + 1) / 2;
			unsigned const index_p90 = (_num_samples * 9 + 9) / 10;
			unsigned       samples   = 0;
			_for_each_bin([&] (unsigned const &bin, unsigned index) {
				if (samples >= _num_samples)
					return;

				Type const bin_start = RANGE_START + (index * _bin_size);
				Type const bin_end   = min(bin_start + _bin_size, RANGE_END);

				auto before = [&] (unsigned const i) {
					return max(min((int)i-(int)samples,(int)bin), 0); };

				unsigned const before_median = before(median);
				unsigned const before_p90    = before(index_p90);

				OUTPUT_FN(Hist_bar(before_median,              quantum, '-'),
				          Hist_bar(before_p90 - before_median, quantum, '='),
				          Hist_bar(bin - before_p90,           quantum, '#'),
				          " ", bin, " in [",
				          Format{bin_start}, ",",
				          Format{bin_end}, "[");

				samples += bin;
			});

			if (_maxed_out) {
				OUTPUT_FN(Hist_bar(_maxed_out, quantum, '~'), " ", _maxed_out,
				          " >= ", RANGE_END);
			}
		}

		Histogram(CONV &conv)
		:
			_conv(conv)
		{
			if ((RANGE_END-RANGE_START)/_bin_size > ~0U)
				error("too many bins for value range, bin index must be unsigned");

			_for_each_bin([&] (unsigned &bin, unsigned) {
				bin = 0; });
		}
};

template <typename STATS>
class Genode::Tsc_hist_probe : Noncopyable
{
	private:

		using Timestamp = Trace::Timestamp;

	public:

		class Scope : Noncopyable
		{
			private:

				friend class Tsc_hist_probe;

				unsigned  _num_entered = 0;  /* recursion depth */
				Mutex     _mutex { };        /* protect scope */

				void enter()
				{
					Mutex::Guard guard(_mutex);

					_num_entered++;
				}

				void leave(STATS &stats, Timestamp duration)
				{
					Mutex::Guard guard(_mutex);

					/*
					 * If the probed scope is executed recursively or
					 * concurrently by multiple threads, defer the accounting
					 * until the scope is completely left.
					 */
					_num_entered--;
					if (_num_entered > 0)
						return;

					stats.add(duration);
				}
		};

	private:

		STATS &_stats;
		Scope  _scope { };

		Timestamp const _start = Trace::timestamp();

	public:

		Tsc_hist_probe(STATS &stats)
		:
			_stats(stats)
		{
			_scope.enter();
		}

		~Tsc_hist_probe()
		{
			_scope.leave(_stats, Trace::timestamp() - _start);
		}
};

#endif /* _TSC_HISTOGRAM_H_ */
