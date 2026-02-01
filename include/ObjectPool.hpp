#pragma once

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

template <class T>
class ObjectPool
{
public:
	// Handle uniquely identifies an object in the pool
	struct Handle
	{
		std::uint32_t index = invalidIndex;
		std::uint32_t generation = 0;

		friend bool operator==(const Handle& a, const Handle& b)
		{
			return a.index == b.index && a.generation == b.generation;
		}

		friend bool operator!=(const Handle& a, const Handle& b)
		{
			return !(a == b);
		}

		static constexpr std::uint32_t invalidIndex = 0xFFFFFFFFu;
	};

	ObjectPool() = default;

	explicit ObjectPool(const std::size_t initialCapacity)
	{
		reserve(initialCapacity);
	}

	void reserve(std::size_t n)
	{
		m_items.reserve(n);
		m_generations.reserve(n);
		m_active.reserve(n);
		m_freeList.reserve(n);
	}

	std::size_t capacity() const
	{
		return m_items.size();
	}

	void clear()
	{
		m_freeList.clear();
		m_freeList.reserve(m_items.size());

		for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(m_items.size()); ++i)
		{
			if (m_active[i])
			{
				m_active[i] = 0;
				m_generations[i]++;
			}
			m_freeList.push_back(i);
		}
	}

	// Acquires a new or recycled object, returns its handle
	template <class... Args>
	Handle acquire(Args&&... args)
	{
		const std::uint32_t idx = acquireIndex();
		if (idx >= m_items.size())
		{
			m_items.emplace_back(std::forward<Args>(args)...);
			m_generations.push_back(0);
			m_active.push_back(1);
			return Handle{idx, 0};
		}

		m_items[idx] = T(std::forward<Args>(args)...);
		m_active[idx] = 1;
		return Handle{idx, m_generations[idx]};
	}

	// Releases an object, making its slot reusable
	void release(Handle h)
	{
		if (!isValid(h)) return;
		if (!m_active[h.index]) return;

		m_active[h.index] = 0;
		++m_generations[h.index];
		m_freeList.push_back(h.index);
	}

	// Checks if a handle is valid (not released or overwritten)
	bool isValid(Handle h) const
	{
		if (h.index == Handle::invalidIndex) return false;
		if (h.index >= m_items.size()) return false;
		return m_generations[h.index] == h.generation;
	}

	// Checks if a handle is valid and currently active
	bool isActive(Handle h) const
	{
		return isValid(h) && m_active[h.index] != 0;
	}

	T& get(Handle h)
	{
		assert(isActive(h) && "ObjectPool::get: handle invalid or inactive");
		return m_items[h.index];
	}

	const T& get(Handle h) const
	{
		assert(isActive(h) && "ObjectPool::get: handle invalid or inactive");
		return m_items[h.index];
	}

	T* tryGet(Handle h)
	{
		if (!isActive(h)) return nullptr;
		return &m_items[h.index];
	}

	const T* tryGet(Handle h) const
	{
		if (!isActive(h)) return nullptr;
		return &m_items[h.index];
	}

	// Iterates over all active objects
	template <class F>
	void forEachActive(F&& fn)
	{
		for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(m_items.size()); ++i)
		{
			if (!m_active[i]) continue;
			Handle h{i, m_generations[i]};
			fn(m_items[i], h);
		}
	}


	template <class F>
	void forEachActive(F&& fn) const
	{
		for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(m_items.size()); ++i)
		{
			if (!m_active[i]) continue;
			Handle h{i, m_generations[i]};
			fn(m_items[i], h);
		}
	}

private:
	// Returns index for a new or recycled object
	std::uint32_t acquireIndex()
	{
		if (!m_freeList.empty())
		{
			const std::uint32_t idx = m_freeList.back();
			m_freeList.pop_back();
			return idx;
		}
		return static_cast<std::uint32_t>(m_items.size());
	}

	std::vector<T> m_items;
	std::vector<std::uint32_t> m_generations;
	std::vector<std::uint8_t> m_active;
	std::vector<std::uint32_t> m_freeList;
};