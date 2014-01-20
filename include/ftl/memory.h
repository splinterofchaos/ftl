/*
 * Copyright (c) 2013 Björn Aili
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 */
#ifndef FTL_MEMORY_H
#define FTL_MEMORY_H

#include <memory>
#include "concepts/monoid.h"
#include "concepts/monad.h"
#include "concepts/foldable.h"

namespace ftl {

	/**
	 * \defgroup memory Memory
	 *
	 * Concepts instances for std::shared_ptr.
	 *
	 * \code
	 *   #include <ftl/memory.h>
	 * \endcode
	 *
	 * This module adds the following concept instances to std::share_ptr:
	 * - \ref monoid
	 * - \ref functor
	 * - \ref applicative
	 * - \ref monad
	 *
	 * \par Dependencies
	 * - <memory>
	 * - \ref monoid
	 * - \ref monad
	 * - \ref foldable
	 */

	/**
	 * Monoid instance for shared_ptr.
	 *
	 * Much like maybe, any shared_ptr that wraps a monoid is also a monoid.
	 *
	 * \ingroup memory
	 */
	template<typename T>
	struct monoid<std::shared_ptr<T>> {
		/// Simply creates an "empty" pointer.
		static constexpr auto id() noexcept
		-> typename std::enable_if<
				monoid<T>::instance,
				std::shared_ptr<T>>::type {
			return std::shared_ptr<T>();
		}

		/**
		 * Unwraps the values and applies their monoid op.
		 *
		 * Note that if a points to \c a shared object, but not \c b, \c a
		 * is returned, \em not a pointer to a new object that is equal to
		 * whatever \c a points to. Same goes for the revrse situation.
		 *
		 * If neither of the pointers point anywhere, another "empty" pointer
		 * is returned.
		 *
		 * And finally, if both the pointers point to some object, then
		 * \c make_shared is invoked to create a new object that is the result
		 * of applying the monoid operation on the two values.
		 */
		static auto append(
				std::shared_ptr<T> a,
				std::shared_ptr<T> b)
		-> typename std::enable_if<
				monoid<T>::instance,
				std::shared_ptr<T>>::type {
			if(a) {
				if(b)
					return std::make_shared<T>(monoid<T>::append(*a, *b));

				else
					return a;
			}

			else {
				if(b)
					return b;

				else
					return std::shared_ptr<T>();
			}
		}

		/// \c shared_ptr is only a monoid instance if T is.
		static constexpr bool instance = monoid<T>::instance;
	};

	/**
	 * Monoid instance for unique_ptr.
	 *
	 * Much like maybe, any shared_ptr that wraps a monoid is also a monoid.
	 *
	 * \ingroup memory
	 */
	template<typename T>
	struct monoid<std::unique_ptr<T>> {
		/// Simply creates an "empty" pointer.
		static constexpr auto id() noexcept
		-> typename std::enable_if<
				monoid<T>::instance,
				std::unique_ptr<T>>::type {
			return std::unique_ptr<T>();
		}

	private:
		/// Create a valued pointer (in lue of std::make_unique).
		template<typename...Init>
		static constexpr std::unique_ptr<T> make(Init&&...init) {
			return std::unique_ptr<T>(new T(std::forward<Init>(init)...));
		}

		/// Duplicate a pointer. (unique_ptr's copy ctor is deleted.)
		static constexpr std::unique_ptr<T> dup(const std::unique_ptr<T>& p) {
			return make(*p);
		}
	public:

		/**
		 * Unwraps the values and applies their monoid op.
		 *
		 * If neither of the pointers point anywhere, another "empty" pointer
		 * is returned.
		 *
		 * If both the pointers point to some object, then a new pointer with
		 * the result of the monoid operation is created, unless a or b is an
		 * rvalue.
		 */		
		static auto append(
				const std::unique_ptr<T>& a,
				const std::unique_ptr<T>& b)
		-> typename std::enable_if<
				monoid<T>::instance,
				std::unique_ptr<T>>::type {
			if(a) {
				if(b) {
					return make(monoid<T>::append(*a, *b));
				}

				else
					return dup(a);
			}

			else {
				if(b)
					return dup(b);

				else
					return id();
			}
		}
		static auto append(
				std::unique_ptr<T>&& a,
				const std::unique_ptr<T>& b)
		-> typename std::enable_if<
				monoid<T>::instance,
				std::unique_ptr<T>>::type {
			if(b) {
				if(a) {
					*a = monoid<T>::append(std::move(*a), *b);
					return std::move(a);
				}

				else
					return dup(b);
			}

			else {
				// If a, return a, else id.
				return std::move(a);
			}
		}

		static auto append(
				const std::unique_ptr<T>& a,
				std::unique_ptr<T>&& b)
		-> typename std::enable_if<
				monoid<T>::instance,
				std::unique_ptr<T>>::type {
			if(a) {
				if(b) {
					*b = monoid<T>::append(*a, std::move(*b));
					return std::move(b);
				}
				else
					return dup(a);
			}

			else {
				// If b, return b, else id.
				return std::move(b); 
			}
		}

		static auto append(
				std::unique_ptr<T>&& a,
				std::unique_ptr<T>&& b)
		-> typename std::enable_if<
				monoid<T>::instance,
				std::unique_ptr<T>>::type {
			if(a) {
				if(b) 
					*a = monoid<T>::append(std::move(*a), std::move(*b));

				return std::move(a);
			}

			else {
				// If b, return b, else id.
				return std::move(b); 
			}
		}


		/// \c shared_ptr is only a monoid instance if T is.
		static constexpr bool instance = monoid<T>::instance;
	};

	/**
	 * Monad instance of shared_ptr.
	 *
	 * \ingroup memory
	 */
	template<typename T>
	struct monad<std::shared_ptr<T>>
	: deriving_join<in_terms_of_bind<std::shared_ptr<T>>>
	, deriving_apply<in_terms_of_bind<std::shared_ptr<T>>> {

		static std::shared_ptr<T> pure(T&& a) {
			return std::make_shared<T>(std::forward<T>(a));
		}

		template<typename F, typename U = result_of<F(T)>>
		static std::shared_ptr<U> map(F f, std::shared_ptr<T> p) {
			if(p)
				return std::make_shared<U>(f(*p));

			else
				return std::shared_ptr<U>();
		}

		template<
				typename F,
				typename U = typename result_of<F(T)>::element_type
		>
		static std::shared_ptr<U> bind(std::shared_ptr<T> a, F f) {
			if(a)
				return f(*a);

			return std::shared_ptr<U>();
		}

		static constexpr bool instance = true;
	};

	// The default Rebind will not update the second template parameter,
	// std::default_delete, for std::unique_ptr correctly.
	template<typename T>
	struct parametric_type_traits<std::unique_ptr<T>> {

		using value_type = T;

		template<typename U>
		using rebind = std::unique_ptr<U>;
	};

	/**
	 * Monad instance of unique_ptr.
	 *
	 * \ingroup memory
	 */
	template<typename T>
	struct monad<std::unique_ptr<T>>
	: deriving_join<in_terms_of_bind<std::unique_ptr<T>>> 
	, deriving_apply<in_terms_of_bind<std::unique_ptr<T>>> {

		static std::unique_ptr<T> pure(T&& a) {
			return std::unique_ptr<T>(new T(std::move(a)));
		}

		template<typename F, typename U = result_of<F(T)>>
		static std::unique_ptr<U> map(F f, const std::unique_ptr<T>& p) {
			if(p)
				return std::unique_ptr<U>(new U(f(std::move(*p))));

			else
				return std::unique_ptr<U>();
		}

		template<
			typename F, 
			typename U = result_of<F(T)>,
			typename = Requires<std::is_same<T,U>::value>>
		static std::unique_ptr<U> map(F f, std::unique_ptr<T>&& p) {
			if(p)
				*p = f(std::move(*p));
			
			return std::move(p);
		}

		template<
				typename F,
				typename U = typename result_of<F(T)>::element_type
		>
		static std::unique_ptr<U> bind(const std::unique_ptr<T>& a, F f) {
			if(a)
				return f(std::move(*a));

			return std::unique_ptr<U>();
		}

		static constexpr bool instance = true;
	};

	/**
	 * Foldable instance for shared_ptr
	 *
	 * \ingroup memory
	 */
	template<typename T>
	struct foldable<std::shared_ptr<T>>
	: deriving_foldMap<std::shared_ptr<T>>, deriving_fold<std::shared_ptr<T>> {
		template<
				typename Fn,
				typename U,
				typename = Requires<std::is_same<U, result_of<Fn(U,T)>>::value>
		>
		static U foldl(Fn&& fn, U&& z, std::shared_ptr<T> p) {
			if(p) {
				return fn(std::forward<U>(z), *p);
			}

			return z;
		}

		template<
				typename Fn,
				typename U,
				typename = Requires<std::is_same<U, result_of<Fn(T,U)>>::value>
				>
		static U foldr(Fn&& fn, U&& z, std::shared_ptr<T> p) {
			if(p) {
				return fn(std::forward<U>(z), *p);
			}

			return z;
		}

		static constexpr bool instance = true;
	};
}

#endif

