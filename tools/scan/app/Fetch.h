#pragma once

#define FETCH_STATE_MAP(XX)                                                                                            \
	XX(none)                                                                                                           \
	XX(pending)                                                                                                        \
	XX(success)                                                                                                        \
	XX(failed)                                                                                                         \
	XX(skipped)

// Fetch one description file at a time to avoid swamping the TCP stack
struct Fetch {
	enum class State {
#define XX(n) n,
		FETCH_STATE_MAP(XX)
#undef XX
	};

	using States = BitSet<uint8_t, State>;
	static constexpr States completed{State::success | State::failed | State::skipped};

	Urn::Kind kind{};
	String url;
	String root;
	String path;
	String id;
	State state{State::none};
	uint8_t attempts{0};

	Fetch() = default;
	Fetch(const Fetch&) = default;

	Fetch(Urn::Kind kind, const String& url, const String& root, const String& path)
		: kind(kind), url(url), root(root), path(path)
	{
	}

	Fetch(const Urn& urn) : kind(urn.kind), path(urn.toString())
	{
	}

	explicit operator bool() const
	{
		return kind != Urn::Kind::none;
	}

	bool isComplete() const
	{
		return completed[state];
	}

	bool operator==(const Fetch& other)
	{
		return root == other.root && path == other.path;
	}

	Urn urn() const
	{
		return Urn(path);
	}

	String fullPath() const
	{
		String s;
		s += root;
		s += path;
		auto ext = path.lastIndexOf('.');
		auto sep = path.lastIndexOf('/');
		if(ext < 0 || sep > ext) {
			s += ".xml";
		}
		return s;
	}

	static String toString(State state)
	{
		switch(state) {
#define XX(n)                                                                                                          \
	case State::n:                                                                                                     \
		return F(#n);
			FETCH_STATE_MAP(XX)
#undef XX
		default:
			assert(false);
			return nullptr;
		}
	}

	String toString() const
	{
		String s;
		s += '(';
		s += ::toString(kind);
		if(isComplete()) {
			s += ", ";
			s += toString(state);
		}
		if(attempts > 1) {
			s += ", ";
			s += attempts;
			s += _F(" attempts");
		}
		s += ") ";
		if(url) {
			s += url;
			s += F(" -> ");
		}
		s += fullPath();
		return s;
	}
};

class FetchList : public Vector<Fetch>
{
public:
	FetchList(const String& name) : name(name)
	{
	}

	Fetch& add(Fetch f)
	{
		if(f.kind > Urn::Kind::service) {
			f.kind = Urn::Kind::none;
		}

		int i = indexOf(f);
		if(i < 0) {
			f.state = Fetch::State::pending;
			f.attempts = 0;

			if(f.kind == Urn::Kind::service) {
				insertElementAt(f, 0);
				i = 0;
			} else {
				Vector::addElement(f);
				i = count() - 1;
			}

			Serial.print(_F("Queuing '"));
			Serial.println(f.toString());
		}

		return operator[](i);
	}

	Fetch& find(Fetch::States states)
	{
		for(unsigned i = 0; i < count(); ++i) {
			auto& f = operator[](i);
			if(states[f.state]) {
				return f;
			}
		}

		static Fetch nil;
		nil = Fetch();
		return nil;
	}

	using Vector::count;

	unsigned count(Fetch::States states) const
	{
		unsigned n{0};
		for(unsigned i = 0; i < count(); ++i) {
			auto& f = elementAt(i);
			if(states[f.state]) {
				++n;
			}
		}

		return n;
	}

	String toString() const
	{
		String s;
		s += name;
		s += F(": completed ");
		s += count(Fetch::completed);
		s += F(" of ");
		s += count();
		s += ", ";
		s += count(Fetch::State::skipped);
		s += F(" skipped, ");
		s += count(Fetch::State::failed);
		s += F(" failed.");
		return s;
	}

private:
	String name;
};

inline String toString(Fetch::State state)
{
	return Fetch::toString(state);
}
